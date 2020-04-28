
#include "menubar.h"

#include "containers_pane.h"

#include <gtk/gtk.h>

bool advanced_view = false;
bool advanced_view_is_on() {
    return advanced_view;
}

void File_item_New_func(GtkMenuItem *menuitem, gpointer data) {
    printf("New!\n");
}

void toggle_advanced_view_func(GtkWidget *widget, gpointer statusbar) {
    if (gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
        advanced_view = true;
    } else {
        advanced_view = false;
    }

    containers_refilter_model();
}

// TODO: should do this layout in xml/glade
GtkWidget* menubar_init() {
    GtkWidget* menubar = gtk_menu_bar_new();

    // NOTE: this item/menu/item stuff is dumb
    // Create top level "File" menu item
    GtkWidget* File_item = gtk_menu_item_new_with_label("File");
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), File_item);
    // Add drop-down menu to "File" item
    GtkWidget* File_menu = gtk_menu_new();
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(File_item), File_menu);

    // New item
    GtkWidget* File_item_New = gtk_menu_item_new_with_label("New");
    g_signal_connect(G_OBJECT(File_item_New), "activate", G_CALLBACK(File_item_New_func), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(File_menu), File_item_New);

    // Advanced view toggle
    // TODO: menu is closed when this is toggled, which feels weird, find a way to keep it open
    GtkWidget* toggle_advanced_view = gtk_check_menu_item_new_with_label("Advanced view");
    gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(toggle_advanced_view), advanced_view);
    g_signal_connect(G_OBJECT(toggle_advanced_view), "activate", G_CALLBACK(toggle_advanced_view_func), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(File_menu), toggle_advanced_view);

    // Quit item
    GtkWidget* File_item_Quit = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(G_OBJECT(File_item_Quit), "activate", G_CALLBACK(gtk_main_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(File_menu), File_item_Quit);

    return menubar;
}