
#include "menu_bar.h"
#include "containers_view.h"
#include "contents_view.h"
#include "entry.h"
#include "utils.h"

bool advanced_view = false;

GtkDialog* new_user_dialog = NULL;
GtkEntry* new_user_name_entry = NULL;

bool advanced_view_is_on() {
    return advanced_view;
}

void advanced_view_toggled_func(GtkWidget *widget, gpointer data) {
    advanced_view = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget));

    containers_refilter();
    contents_refilter();
}

void menu_bar_file_new_user_func(GtkWidget *widget, gpointer data) {
    // TODO: set max chars for entry (in glade) based on ldap limits
    int response = gtk_dialog_run(new_user_dialog);
    
    if (response == GTK_RESPONSE_OK) {
        printf("new user is %s\n", gtk_entry_get_text(new_user_name_entry));

        entry_new(gtk_entry_get_text(new_user_name_entry), NewEntryType_User);

        gtk_entry_set_text(new_user_name_entry, "");
        gtk_widget_hide(GTK_WIDGET(new_user_dialog));
    } else if (response == GTK_RESPONSE_CANCEL) {
        gtk_entry_set_text(new_user_name_entry, "");
        gtk_widget_hide(GTK_WIDGET(new_user_dialog));
    } else {
        printf("ERROR: menu_bar_file_new_user_func() received unknown button response %d !\n", response);
    }
}

void menu_bar_init(GtkBuilder* builder) {
    new_user_dialog = GTK_DIALOG(gtk_builder_get_object_CHECKED(builder, "new_user_dialog"));
    new_user_name_entry = GTK_ENTRY(gtk_builder_get_object_CHECKED(builder, "new_user_dialog_name_entry"));
}