#pragma once

#include <gtk/gtk.h>

void contents_init(GtkBuilder* builder);
void contents_change_target(const char* new_target_dn);
void contents_populate_model();
void contents_refilter();
void contents_selection_changed_func(GtkTreeSelection* selection, gpointer user_data);