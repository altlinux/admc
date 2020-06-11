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
int dn2domain(const char* dn, char** domain);

static void test_dn2domain_correct(void **state) {
    const char* test_dn = "DC=domain,DC=alt";
    const char* desired_result = "domain.alt.";
    char* domain = NULL;
    int result = dn2domain(test_dn, &domain);

    assert_return_code(result, AD_SUCCESS);
    assert_non_null(domain);
    assert_string_equal(domain, desired_result);

    if (NULL != domain) {
        free(domain);
        domain = NULL;
    }
}

static void test_dn2domain_incorrect(void **state) {
    const char* test_dn = "some-shit";
    char* domain = NULL;
    int result = dn2domain(test_dn, &domain);

    assert_return_code(result, AD_LDAP_OPERATION_FAILURE);
    assert_null(domain);

    if (NULL != domain) {
        free(domain);
        domain = NULL;
    }
}

int main(void) {
    const struct CMUnitTest tests[] = {
          cmocka_unit_test(test_dn2domain_correct)
        , cmocka_unit_test(test_dn2domain_incorrect)
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}

