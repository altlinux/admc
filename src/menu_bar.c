
#include "menu_bar.h"
#include "containers_view.h"
#include "contents_view.h"

#include <gtk/gtk.h>

bool advanced_view = false;

bool advanced_view_is_on() {
    return advanced_view;
}

void advanced_view_toggled_func(GtkWidget *widget, gpointer statusbar) {
    advanced_view = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));

    containers_refilter();
    contents_refilter();
}
