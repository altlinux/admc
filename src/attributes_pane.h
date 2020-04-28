#pragma once

#include <gtk/gtk.h>

GtkWidget* attributes_init();
void attributes_update_model(const char* new_root_dn);
