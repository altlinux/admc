
#include "contents_pane.h"

#include "attributes_pane.h"
#include "entry.h"
#include "constants.h"

#include "stb_ds.h"
#include <gtk/gtk.h>

enum {
    CONTENTS_COLUMN_DN,
    CONTENTS_COLUMN_NAME,
    CONTENTS_COLUMN_CATEGORY,
    CONTENTS_COLUMN_DESCRIPTION,
    CONTENTS_COLUMN_COUNT
};

GtkWidget* contents_view = NULL;

void contents_selection_changed_func(GtkTreeSelection* selection, gpointer user_data) {
    GtkTreeView* view = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel* model = gtk_tree_view_get_model(view);

    GtkTreeIter iter;

    gboolean any_selected = gtk_tree_selection_get_selected(selection, &model, &iter);
    if (!any_selected) {
        return;
    }

    // Get dn of selected iter
    char* dn;
    gtk_tree_model_get(model, &iter, CONTENTS_COLUMN_DN, &dn, -1);

    // Update contents model
    attributes_update_model(dn);

    free(dn);
}

void contents_update_model(const char* new_root_dn) {
    GtkTreeStore* model = gtk_tree_store_new(CONTENTS_COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

    entry* e = shget(entries, new_root_dn);

    // Populate model
    for (int i = 0; i < arrlen(e->children); i++) {
        char* child_dn = e->children[i];
        entry* child = shget(entries, child_dn);

        GtkTreeIter this_node;
        gtk_tree_store_append(model, &this_node, NULL);

        // DN
        gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_DN, child->dn, -1);

        // Name
        STR_ARRAY name = entry_get_attribute(child, "name");
        gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_NAME, name[0], -1);

        // Category
        STR_ARRAY category_dn = entry_get_attribute(child, "objectCategory");
        if (category_dn == NULL) {
            gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_DESCRIPTION, "none", -1);
        } else {
            char short_category[DN_LENGTH_MAX];
            first_element_in_dn(short_category, category_dn[0], DN_LENGTH_MAX);
            gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_CATEGORY, short_category, -1);
        }

        // Description
        STR_ARRAY description = entry_get_attribute(child, "description");
        if (description == NULL) {
            gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_DESCRIPTION, "none", -1);
        } else {
            gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_DESCRIPTION, description[0], -1);
        }
    }

    gtk_tree_view_set_model(GTK_TREE_VIEW(contents_view), GTK_TREE_MODEL(model));
    g_object_unref(model);
}

GtkWidget* contents_init() {
    // NOTE: contents model is populated when a container in containers panel is selected
    contents_view = gtk_tree_view_new();

    GtkTreeViewColumn* dn_column = gtk_tree_view_column_new_with_attributes("DN", gtk_cell_renderer_text_new(), "text", CONTENTS_COLUMN_DN, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(contents_view), dn_column);
    gtk_tree_view_column_set_visible(dn_column, FALSE);

    GtkTreeViewColumn* name_column = gtk_tree_view_column_new_with_attributes("Name", gtk_cell_renderer_text_new(), "text", CONTENTS_COLUMN_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(contents_view), name_column);
    gtk_tree_view_column_set_sizing(name_column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(name_column, 200);

    GtkTreeViewColumn* category_column = gtk_tree_view_column_new_with_attributes("Category", gtk_cell_renderer_text_new(), "text", CONTENTS_COLUMN_CATEGORY, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(contents_view), category_column);
    gtk_tree_view_column_set_sizing(category_column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(category_column, 200);

    GtkTreeViewColumn* description_column = gtk_tree_view_column_new_with_attributes("Description", gtk_cell_renderer_text_new(), "text", CONTENTS_COLUMN_DESCRIPTION, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(contents_view), description_column);
    gtk_tree_view_column_set_sizing(description_column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(description_column, 300);

    // Selection changed callback
    GtkTreeSelection* tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(contents_view));
    g_signal_connect(tree_selection, "changed", G_CALLBACK(contents_selection_changed_func), NULL);

    return contents_view;
}
