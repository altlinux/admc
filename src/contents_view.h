#pragma once

#include <gtk/gtk.h>

void contents_init(GtkBuilder* builder);
void contents_populate_model(const char* new_root);
void contents_refilter();
