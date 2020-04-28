
#include "mainwindow.h"
#include "entry.h"
#include "constants.h"

// NOTE: need to define this once in one .c file
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"
#include "active_directory.h"
#include <stdbool.h>
#include <gtk/gtk.h>

// entries_map* entries;

// void add_entry(const char* dn, const char* parent_dn) {
//     entry* e = (entry*)malloc(sizeof(entry));
//     e->dn = strdup(dn);
//     e->children = NULL;
//     shput(entries, dn, e);

//     if (parent_dn != NULL) {
//         // Add entry to parent's children list
//         entry* parent = shget(entries, parent_dn);

//         if (parent != NULL) {
//             arrput(parent->children, strdup(dn));
//         } else {
//             printf("add_entry(): given parent_dn doesn't exist in entries map!");
//         }
//     }
// }

int main(int argc, char** argv) { 
    // TODO: create window and show a loading bar before doing all this stuff?

    LDAP* ldap_connection = ad_login();
    if (ldap_connection == NULL) {
        printf("ad_login error: %s\n", ad_get_error());
        return 0;
    }

    // Init entries map
    sh_new_strdup(entries);

    // Load all entries recursively
    entry_load(HEAD_DN); 

    // // Setup up data
    // {
    //     sh_new_strdup(entries);
        
    //     add_entry("HEAD", NULL);
    //     add_entry("Dave", "HEAD");
    //     add_entry("Mary", "HEAD");
    //     add_entry("Alice", "HEAD");

    //     add_entry("Dave's son", "Dave");
    //     add_entry("Dave's daughter", "Dave");

    //     add_entry("Mary's son", "Mary");
    //     add_entry("Mary's daughter", "Mary");
    //     add_entry("Mary's pc", "Mary");

    //     add_entry("Alice's son", "Alice");
    //     add_entry("Alice's daughter", "Alice");
    //     add_entry("Alice's book", "Alice");

    //     add_entry("Alice's daughter's teddy bear", "Alice's daughter");
    // }

    // Setup UI
    gtk_init(&argc, &argv);
    GtkWidget* window = mainwindow_init();
    gtk_widget_show_all(window);
    gtk_main();

    return 0;
}
