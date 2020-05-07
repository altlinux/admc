
#include "attributes_view.h"

#include "constants.h"
#include "utils.h"
#include "entry.h"

#include "stb_ds.h"
#include <gtk/gtk.h>

// Lists attributes of target entry
// Attributes target entry is selected in contents view

enum {
    ATTRIBUTES_COLUMN_NAME,
    ATTRIBUTES_COLUMN_VALUE,
    ATTRIBUTES_COLUMN_COUNT
};

GtkTreeView* attributes_view = NULL;
char* attributes_target = NULL;

// NOTE: currently doesn't work for multi-valued attributes, add that capability when/if it's needed
void attributes_value_edited_func(
    GtkCellRendererText* cell,
    gchar* path_string,
    gchar* new_text,
    gpointer user_data)
{
    GtkTreeModel* model = gtk_tree_view_get_model(attributes_view);

    GtkTreeIter iter;
    gtk_tree_model_get_iter_from_string(model, &iter, path_string);

    char* attribute_name;
    gtk_tree_model_get(model, &iter, ATTRIBUTES_COLUMN_NAME, &attribute_name, -1);

    char* old_value;
    gtk_tree_model_get(model, &iter, ATTRIBUTES_COLUMN_VALUE, &old_value, -1);

    entry* e = get_entry(attributes_target);
    bool edit_success = entry_edit_value(e, attribute_name, new_text);

    if (edit_success) {
        gtk_tree_store_set(GTK_TREE_STORE(model), &iter, ATTRIBUTES_COLUMN_VALUE, new_text, -1);

        printf("attribute %s changed: %s > %s\n", attribute_name, old_value, new_text);
    } else {
        // TODO: display error
    }

    free(attribute_name);
    free(old_value);
}

void attributes_change_target(const char* new_target_dn) {
    if (attributes_target != NULL) {
        free(attributes_target);
    }
    attributes_target = strdup(new_target_dn);
}

// NOTE: model is set when an entry is selected in contents pane
void attributes_populate_model() {
    // Populate model
    // List all key->value pairs in order
    GtkTreeStore* model = GTK_TREE_STORE(gtk_tree_view_get_model(attributes_view));
    gtk_tree_store_clear(model);

    entry* e = get_entry(attributes_target);

    // Target is invalid
    // NOTE: this is valid behavior and can occur when target entry is deleted for example
    if (e == NULL) {
        return;
    }

    STR_ARRAY attribute_keys = e->attribute_keys;
    for (int i = 0; i < arrlen(attribute_keys); i++) {
        char* key = attribute_keys[i];
        STR_ARRAY values = shget(e->attributes, key);

        for (int j = 0; j < arrlen(values); j++) {
            GtkTreeIter this_node;
            gtk_tree_store_append(model, &this_node, NULL);
            gtk_tree_store_set(model, &this_node, ATTRIBUTES_COLUMN_NAME, key, -1);
            gtk_tree_store_set(model, &this_node, ATTRIBUTES_COLUMN_VALUE, values[j], -1);

            // TODO: handle this, can't draw this text properly
            if (streql(key, "objectGUID")) {
                gtk_tree_store_set(model, &this_node, ATTRIBUTES_COLUMN_VALUE, "!!!can't render this for now", -1);
            }
        }
    }
}

void attributes_init(GtkBuilder* builder) {
    attributes_view = GTK_TREE_VIEW(gtk_builder_get_object_CHECKED(builder, "attributes_view"));
}
