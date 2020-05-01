#pragma once

#include <gtk/gtk.h>

void attributes_init(GtkBuilder* builder);
void attributes_populate_model(const char* new_root_dn);
