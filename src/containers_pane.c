
#include "containers_pane.h"

#include "contents_pane.h"
#include "menubar.h"
#include "constants.h"
#include "utils.h"
#include "entry.h"

#include "stb_ds.h"
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>

enum {
    CONTAINERS_COLUMN_DN,
    CONTAINERS_COLUMN_NAME,
    CONTAINERS_COLUMN_COUNT
};

GtkWidget* containers_view = NULL;

void containers_refilter_model() {
    GtkTreeModel* model_filter = gtk_tree_view_get_model(GTK_TREE_VIEW(containers_view));
    gtk_tree_model_filter_refilter(GTK_TREE_MODEL_FILTER(model_filter));
}

void containers_populate_model(GtkTreeStore* model, char* node_dn, GtkTreeIter* parent) {
    // Populate model with name's of entries
    
    entry* e = shget(entries, node_dn);

    // Skip if entry is not a container
    if (!entry_is_container(e)) {
        return;
    }

    // TODO: handle no name?
    STR_ARRAY name = entry_get_attribute(e, "name");

    GtkTreeIter this_node;
    gtk_tree_store_append(model, &this_node, parent);
    gtk_tree_store_set(model, &this_node, CONTAINERS_COLUMN_DN, node_dn, -1);
    gtk_tree_store_set(model, &this_node, CONTAINERS_COLUMN_NAME, name[0], -1);

    // Recurse into entry's children
    for (int i = 0; i < arrlen(e->children); i++) {
        char* child_dn = e->children[i];

        containers_populate_model(model, child_dn, &this_node);
    }
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
    contents_update_model(dn);

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

GtkWidget* containers_init() {
    // 2 columns: name and dn
    // only name is shown, dn is used for identification in control

    // Create and populate child model
    GtkTreeStore* child_model = gtk_tree_store_new(CONTAINERS_COLUMN_COUNT, G_TYPE_STRING, G_TYPE_STRING);
    containers_populate_model(child_model, HEAD_DN, NULL);

    // Create model filter
    GtkTreeModel* model = gtk_tree_model_filter_new(GTK_TREE_MODEL(child_model), NULL);
    gtk_tree_model_filter_set_visible_func(GTK_TREE_MODEL_FILTER(model), containers_filter_func, NULL, NULL);

    containers_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(model));
    g_object_unref(model);

    // DN column, invisible, only for identification
    GtkTreeViewColumn* dn_column = gtk_tree_view_column_new_with_attributes("DN", gtk_cell_renderer_text_new(), "text", CONTAINERS_COLUMN_DN, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(containers_view), dn_column);
    gtk_tree_view_column_set_visible(dn_column, FALSE);

    // Name column
    GtkTreeViewColumn* name_column = gtk_tree_view_column_new_with_attributes("Name", gtk_cell_renderer_text_new(), "text", CONTAINERS_COLUMN_NAME, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(containers_view), name_column);
    gtk_tree_view_column_set_sizing(name_column, GTK_TREE_VIEW_COLUMN_FIXED);
    gtk_tree_view_column_set_fixed_width(name_column, 300);

    // Selection changed callback
    GtkTreeSelection* tree_selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(containers_view));
    g_signal_connect(tree_selection, "changed", G_CALLBACK(containers_selection_changed_func), NULL);

    return containers_view;
}
