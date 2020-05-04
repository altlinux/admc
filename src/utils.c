
#include "utils.h"

#include <string.h>
#include <stdbool.h>

bool streql(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}

// Use this instead of regular gtk_builder_get_object() so that
// on error the object's name is printed
GObject* gtk_builder_get_object_CHECKED(GtkBuilder* builder, const char *name) {
    GObject* object = gtk_builder_get_object(builder, name);

    if (object != NULL) {
        return object;
    } else {
        printf("gtk_builder_get_object() failed to get \"%s\"", name);
        return NULL;
    }
}
