
#include "mainwindow.h"

#include "menubar.h"
#include "containers_pane.h"
#include "contents_pane.h"
#include "attributes_pane.h"

#include <gtk/gtk.h>

// TODO: good behavior would be 3 panes, border adjustable between each
// none of them auto-resize, start out at reasonable widths for each column
GtkWidget* mainwindow_init() {
    GtkWidget* menubar = menubar_init();
    GtkWidget* containers_view = containers_init();
    GtkWidget* contents_view = contents_init();
    GtkWidget* attributes_view = attributes_init();
    
    // Horizontal box containing panes
    // Expand and fill each pane
    GtkWidget* panes = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    {
        GtkWidget* frame1 = gtk_frame_new("Containers");
        GtkWidget* frame2 = gtk_frame_new("Contents");
        gtk_frame_set_shadow_type(GTK_FRAME(frame1), GTK_SHADOW_IN);
        gtk_frame_set_shadow_type(GTK_FRAME(frame2), GTK_SHADOW_IN);

        gtk_widget_set_size_request(panes, 200, -1);

        // gtk_paned_pack1(GTK_PANED(panes), frame1, true, false);
        // gtk_widget_set_size_request(frame1, 50, -1);
        gtk_container_add(GTK_CONTAINER(frame1), containers_view);

        // gtk_paned_pack2(GTK_PANED(panes), frame2, true, FALSE);
        // gtk_widget_set_size_request(frame2, 50, -1);
        gtk_container_add(GTK_CONTAINER(frame2), contents_view);

        gtk_paned_add1(panes, frame1);
        gtk_paned_add2(panes, frame2);
        gtk_paned_set_wide_handle(panes, true);
    }

    // int box_spacing = 0;
    // GtkWidget* box_for_panes = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, box_spacing);
    // gtk_box_pack_start(GTK_BOX(box_for_panes), panes, false, false, 0);
    // gtk_box_pack_start(GTK_BOX(box_for_panes), attributes_view, true, true, 0);

    // // Scrolled window
    // GtkWidget* scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    // gtk_container_add(GTK_CONTAINER(scrolled_window), box_for_panes);

    // Window
    GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_container_add(GTK_CONTAINER(window), panes);

    gtk_window_set_title(GTK_WINDOW(window), "Adtool");

    // Set starting window size
    gtk_window_set_default_size(GTK_WINDOW(window), 1700, 900);

    // Set minimum window size
    gtk_widget_set_size_request(window, 1500, 900);

    return window;
}
