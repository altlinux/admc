
#include "mainwindow.h"
#include "entry.h"

#include "active_directory.h"
#include <gtk/gtk.h>

int main(int argc, char** argv) { 
    // TODO: show some kind of progress indicator before loading data? it's not instant

    bool fake_entries = false;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "fake") == 0) {
            fake_entries = true;
        }
    }

    if (fake_entries) {
        entry_init_fake();
    } else {
        // Login into ldap
        LDAP* ldap_connection = ad_login();
        if (ldap_connection == NULL) {
            printf("ad_login error: %s\n", ad_get_error());
            return 0;
        }

        // Load entry data
        entry_init();
    }


    // Setup UI
    gtk_init(&argc, &argv);
    GtkWidget* window = mainwindow_init();
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
