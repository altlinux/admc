
#include "contents_view.h"

#include "attributes_view.h"
#include "entry.h"
#include "constants.h"
#include "menu_bar.h"
#include "utils.h"

#include "stb_ds.h"
#include <gtk/gtk.h>

enum {
    CONTENTS_COLUMN_DN,
    CONTENTS_COLUMN_NAME,
    CONTENTS_COLUMN_CATEGORY,
    CONTENTS_COLUMN_DESCRIPTION,
    CONTENTS_COLUMN_COUNT
};

GtkTreeView* contents_view = NULL;

void contents_refilter() {
    GtkTreeModelFilter* model_filter = GTK_TREE_MODEL_FILTER(gtk_tree_view_get_model(contents_view));
    gtk_tree_model_filter_refilter(model_filter);
}

gboolean contents_filter_func(
    GtkTreeModel *model,
    GtkTreeIter  *iter,
    gpointer      data)
{
    // Filter out entries with "showInAdvancedViewOnly"  set to TRUE
    gboolean visible = TRUE;

    char* dn;
    gtk_tree_model_get(model, iter, CONTENTS_COLUMN_DN, &dn, -1);
    if (dn != NULL) {
        entry* e = shget(entries, dn);
        STR_ARRAY showInAdvancedViewOnly = entry_get_attribute(e, "showInAdvancedViewOnly");
        if (!advanced_view_is_on() && showInAdvancedViewOnly != NULL && streql(showInAdvancedViewOnly[0], "TRUE")) {
            visible = FALSE;
        }

        g_free(dn);
    }

    return visible;
}

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
    attributes_populate_model(dn);

    free(dn);
}

// NOTE: contents model is repopulated everytime a new container is selected
void contents_populate_model(const char* new_root_dn) {
    GtkTreeModelFilter* model_filter = GTK_TREE_MODEL_FILTER(gtk_tree_view_get_model(contents_view));
    GtkTreeStore* model = GTK_TREE_STORE(gtk_tree_model_filter_get_model(model_filter));
    gtk_tree_store_clear(model);

    entry* e = shget(entries, new_root_dn);

    // Populate model
    for (int i = 0; i < arrlen(e->children); i++) {
        char* child_dn = e->children[i];
        entry* child = shget(entries, child_dn);

        GtkTreeIter this_node;
        gtk_tree_store_append(model, &this_node, NULL);

        char* dn = child->dn;
        gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_DN, dn, -1);

        char name[DN_LENGTH_MAX];
        first_element_in_dn(name, dn, DN_LENGTH_MAX);
        gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_NAME, name, -1);

        char* category_dn = entry_get_attribute_or_none(child, "objectCategory");
        char category[DN_LENGTH_MAX];
        first_element_in_dn(category, category_dn, DN_LENGTH_MAX);
        gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_CATEGORY, category, -1);

        char* description = entry_get_attribute_or_none(child, "description");
        gtk_tree_store_set(model, &this_node, CONTENTS_COLUMN_DESCRIPTION, description, -1);
    }
}

void contents_init(GtkBuilder* builder) {
    contents_view = GTK_TREE_VIEW(gtk_builder_get_object_CHECKED(builder, "contents_view"));

    // Set filter func
    GtkTreeModelFilter* model_filter = GTK_TREE_MODEL_FILTER((gtk_tree_view_get_model(contents_view)));
    gtk_tree_model_filter_set_visible_func(model_filter, contents_filter_func, NULL, NULL);
    gtk_tree_model_filter_refilter(model_filter);
}
