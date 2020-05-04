
#include "containers_view.h"

#include "contents_view.h"
#include "constants.h"
#include "utils.h"
#include "entry.h"
#include "menu_bar.h"

#include "stb_ds.h"
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>

enum {
    CONTAINERS_COLUMN_DN,
    CONTAINERS_COLUMN_NAME,
};

GtkTreeView* containers_view = NULL;

void containers_refilter() {
    GtkTreeModelFilter* model_filter = GTK_TREE_MODEL_FILTER(gtk_tree_view_get_model(containers_view));
    gtk_tree_model_filter_refilter(model_filter);
}

void containers_selection_changed_func(GtkTreeSelection* selection, gpointer user_data) {
    // Get selected iter
    GtkTreeView* view = gtk_tree_selection_get_tree_view(selection);
    GtkTreeModel* model = gtk_tree_view_get_model(view);
    GtkTreeIter iter;
    gboolean any_selected = gtk_tree_selection_get_selected(selection, &model, &iter);
    if (!any_selected) {
        return;
    }
    
    // Get dn of selected iter
    char* dn;
    gtk_tree_model_get(model, &iter, CONTAINERS_COLUMN_DN, &dn, -1);

    // Update contents model
    contents_populate_model(dn);

    free(dn);
}

gboolean containers_filter_func(
    GtkTreeModel *model,
    GtkTreeIter  *iter,
    gpointer      data)
{
    // Filter out entries with "showInAdvancedViewOnly"  set to TRUE
    gboolean visible = TRUE;

    char* dn;
    gtk_tree_model_get(model, iter, CONTAINERS_COLUMN_DN, &dn, -1);

    entry* e = shget(entries, dn);
    STR_ARRAY showInAdvancedViewOnly = entry_get_attribute(e, "showInAdvancedViewOnly");
    if (!advanced_view_is_on() && showInAdvancedViewOnly != NULL && streql(showInAdvancedViewOnly[0], "TRUE")) {
        visible = FALSE;
    }

    g_free(dn);

    return visible;
}

void containers_populate_model_recursive(GtkTreeStore* model, char* node_dn, GtkTreeIter* parent) {
    // Populate model with name's of entries

    entry* e = shget(entries, node_dn);

    // Skip if entry is not a container
    // TODO: move this to filter?
    // TODO: make containers and contents share model?
    // contents can use "gtk_tree_path_is_descendant()"
    if (!entry_is_container(e)) {
        return;
    }

    GtkTreeIter this_node;
    gtk_tree_store_append(model, &this_node, parent);

    gtk_tree_store_set(model, &this_node, CONTAINERS_COLUMN_DN, node_dn, -1);

    char name[DN_LENGTH_MAX];
    first_element_in_dn(name, node_dn, DN_LENGTH_MAX);
    gtk_tree_store_set(model, &this_node, CONTAINERS_COLUMN_NAME, name, -1);

    // Recurse into entry's children
    for (int i = 0; i < arrlen(e->children); i++) {
        char* child_dn = e->children[i];

        containers_populate_model_recursive(model, child_dn, &this_node);
    }
}

void containers_init(GtkBuilder* builder) {
    containers_view = GTK_TREE_VIEW(gtk_builder_get_object_CHECKED(builder, "containers_view"));

    // Populate model
    GtkTreeStore* model = GTK_TREE_STORE(gtk_builder_get_object_CHECKED(builder, "containers_model"));
    containers_populate_model_recursive(model, HEAD_DN, NULL);

    // Set filter func
    GtkTreeModelFilter* model_filter = GTK_TREE_MODEL_FILTER((gtk_tree_view_get_model(containers_view)));
    gtk_tree_model_filter_set_visible_func(model_filter, containers_filter_func, NULL, NULL);
    gtk_tree_model_filter_refilter(model_filter);
}
