#pragma once

#include <gtk/gtk.h>

void attributes_init(GtkBuilder* builder);
void attributes_change_target(const char* new_target_dn);
void attributes_populate_model();
