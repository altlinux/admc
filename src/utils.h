#pragma once

#include <stdbool.h>
#include <gtk/gtk.h>

bool streql(const char* a, const char* b);
GObject* gtk_builder_get_object_CHECKED(GtkBuilder* builder, const char *name);