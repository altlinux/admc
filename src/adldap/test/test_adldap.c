/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>

#include <stdlib.h>

#include "active_directory.h"

/* This function is not present in header files */
size_t ad_array_size(char **array);

static void test_ad_array_size(void **state) {
    char *test_array[] = {"1", "2", "3", NULL};

    size_t size = ad_array_size((char **)test_array);

    assert_true(size == 3);
}

int main(void) {
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ad_array_size)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
