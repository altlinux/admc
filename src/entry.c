
#include "entry.h"

#include "utils.h"
#include "constants.h"

// NOTE: need to define this once in one .c file
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "active_directory.h"
#include <stdio.h>
#include <stdlib.h>

entries_map* entries;

void entry_load(const char* dn) {
    entry* e = (entry*)malloc(sizeof(entry));

    e->dn = strdup(dn);

    //
    // Load entry's attributes
    //
    e->attributes = NULL;
    e->attribute_keys = NULL;
    char** attributes_raw = ad_get_attribute(e->dn, NULL);
    if (attributes_raw != NULL) {
        // Init entry's attributes map
        sh_new_strdup(e->attributes);

        // Load e->attributes
        // attributes_raw is in the form of:
        // char** array of [key, value, value, key, value ...]
        // it's a map of keys to values where keys can map to multiple values
        // transform it into:
        // stb map of [key => [value, value ...], key => [value, value ...] ...]
        for (int i = 0; attributes_raw[i + 2] != NULL; i += 2) {
            char* key = attributes_raw[i];

            // Get current values of this attribute            
            STR_ARRAY values = shget(e->attributes, key);
            // Append new value to values list
            arrput(values, strdup(attributes_raw[i + 1]));
            // Update values in attributes map
            shput(e->attributes, key, values);
        }

        // Load e->attributes_keys, which contains keys in their original ordering
        char* current_key = NULL;
        for (int i = 0; attributes_raw[i + 2] != NULL; i += 2) {
            char* key = attributes_raw[i];

            if (current_key == NULL || !streql(current_key, key)) {
                // Encountered a new key, save it
                arrput(e->attribute_keys, strdup(key));
                current_key = key;
            }
        }

        // Free attributes_raw
        for (int i = 0; attributes_raw[i] != NULL; i++) {
            free(attributes_raw[i]);
        }
        free(attributes_raw);
    }

    //
    // Load children list
    //
    e->children = NULL;
    char** children_dns = ad_list(e->dn);
    if (children_dns != NULL) {
        for (int i = 0; children_dns[i] != NULL; i++) {
            char* child_dn = children_dns[i];
            arrput(e->children, strdup(child_dn));

            // Preload children
            entry_load(child_dn);
        }

        free(children_dns);
    }

    // Add entry to entries map
    shput(entries, e->dn, e);
}

void entry_init() {
    // Init map to use strdup for string allocation
    sh_new_strdup(entries);

    // Load entries recursively
    entry_load(HEAD_DN);
}

STR_ARRAY entry_get_attribute(entry* e, const char* key) {
    return shget(e->attributes, key);
}

bool entry_attribute_exists(entry* e, const char* key, const char* value) {
    STR_ARRAY values = shget(e->attributes, key);

    if (values == NULL) {
        return false;
    } else {
        for (int i = 0; i < arrlen(values); i++) {
            if (streql(values[i], value)) {
                return true;
            }
        }

        return false;
    }
}

void entry_delete(entry* e) {
    int result = ad_object_delete(e->dn);

    if (result == AD_SUCCESS) {
        // Delete entry from parent's child list
        // Get parent dn by cutting part before first comma
        char* dn = e->dn;
        const char* comma = strchr(dn, ',');
        char parent_dn[DN_LENGTH_MAX];
        strncpy(parent_dn, comma, DN_LENGTH_MAX);

        entry* parent = shget(entries, parent_dn);
        if (parent != NULL) {
            // Find child index in children array
            STR_ARRAY children = parent->children;
            int delete_i = -1;
            for (int i = 0; i < arrlen(children); i++) {
                char* child = children[i];
                if (streql(child, dn)) {
                    delete_i = i;
                    break;
                }
            }

            if (delete_i != -1) {
                arrdel(children, delete_i);
            }
        }

        // Delete entry from entries map
        // TODO: uhh, do i need to call arrfree on e's members?
        shdel(entries, e->dn);
    } else {
        printf("ad_object_delete error: %s\n", ad_get_error());
    }
}

// TODO: probably need to add more objectClasses
bool entry_is_container(entry* e) {
    static const char* container_objectClasses[4] = {"container", "organizationalUnit", "builtinDomain", "domain"};
    int container_objectClasses_size = sizeof(container_objectClasses) / sizeof(container_objectClasses[0]);
    
    for (int i = 0; i < container_objectClasses_size; i++) {
        if (entry_attribute_exists(e, "objectClass", container_objectClasses[i])) {
            return true;
        }
    }

    return false;
}

// "OU=Something,CN=Blah,CN=Bleh" => "Something"
void first_element_in_dn(char* buffer, const char* dn, size_t buffer_size) {
    if (buffer == NULL || dn == NULL) {
        return;
    }

    // Remove part before first "="
    const char* equals_ptr = strchr(dn, '=');
    if (equals_ptr == NULL || strlen(equals_ptr) <= 1) {
        return;
    }
    strncpy(buffer, equals_ptr + 1, buffer_size);

    // Remove part after first ","
    char* comma_ptr = strchr(buffer, ',');
    if (comma_ptr == NULL) {
        return;
    }
    *comma_ptr = '\0';
}

// NOTE: doesn't handle multi-valued for now
bool entry_edit_value(entry* e, char* key, char* new_value) {
    int result = ad_mod_replace(e->dn, key, new_value);

    STR_ARRAY values = entry_get_attribute(e, key);

    if (result == AD_SUCCESS) {
        free(values[0]);
        values[0] = strdup(new_value);

        return true;
    } else {
        printf("ad_mod_replace error: %s\n", ad_get_error());

        return false;
    }
}
