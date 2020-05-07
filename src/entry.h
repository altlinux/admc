
#ifndef ENTRY_H
#define ENTRY_H

#include <stdbool.h>
#include <stddef.h>

// for use with stb array functions
typedef char** STR_ARRAY;

typedef struct attributes_map {
    char* key;
    STR_ARRAY value;
} attributes_map;

typedef struct entry {
    char* dn;
    attributes_map* attributes;
    STR_ARRAY attribute_keys;
    STR_ARRAY children;
} entry;

typedef struct entries_map {
    char* key;
    entry* value;
} entries_map;

typedef enum {
    NewEntryType_User,
    NewEntryType_Computer,
    NewEntryType_OU,
    NewEntryType_Group
} NewEntryType;

void entry_init();
void entry_init_fake();
void entry_load(const char* dn);
STR_ARRAY entry_get_attribute(entry* e, const char* key);
char* entry_get_attribute_or_none(entry* e, const char* key);
bool entry_new(const char* name, NewEntryType type);
entry* get_entry(const char* dn);
void entry_delete(entry* e);
bool entry_attribute_exists(entry* e, const char* key, const char* value);
bool entry_is_container(entry* e);
void first_element_in_dn(char* buffer, const char* dn, size_t buffer_size);
bool entry_edit_value(entry* e, char* key, char* new_value);

#endif
