
#include "utils.h"

#include <string.h>
#include <stdbool.h>

bool streql(const char* a, const char* b) {
    return strcmp(a, b) == 0;
}
