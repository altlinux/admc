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

#include "admc_test_gplink.h"

#include "gplink.h"

void test_gplink_equality(const Gplink &a, const QString &b);

void ADMCTestGplink::initTestCase() {

}

void ADMCTestGplink::cleanupTestCase() {

}

void ADMCTestGplink::init() {

}

void ADMCTestGplink::cleanup() {

}

void ADMCTestGplink::test_to_string() {
    const QString gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;0][LDAP://UPPER;0]";
    Gplink gplink(gplink_string);

    QVERIFY((gplink.to_string() == gplink_string));
}

void ADMCTestGplink::test_get_gpos() {
    const QString gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;0][LDAP://UPPER;0]";
    Gplink gplink(gplink_string);

    const QList<QString> gpos = gplink.get_gpos();
    const QList<QString> correct_gpos = {"a", "b", "c", "UPPER"};

    QVERIFY((gpos == correct_gpos));
}

void ADMCTestGplink::test_add() {
    const QString gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;0][LDAP://UPPER;0]";
    Gplink gplink(gplink_string);

    // Simple add
    gplink.add("added gpo");

    // Test that case is preserved
    gplink.add("added gpo UPPERCASE");

    // Test that duplicates are ignored
    gplink.add("a");

    const QString correct_gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;0][LDAP://UPPER;0][LDAP://added gpo;0][LDAP://added gpo UPPERCASE;0]";

    test_gplink_equality(gplink, correct_gplink_string);
}

void ADMCTestGplink::test_remove() {
    const QString gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;0][LDAP://UPPER;0]";
    Gplink gplink(gplink_string);

    gplink.remove("b");

    gplink.remove("non existing gpo");

    const QString correct_gplink_string = "[LDAP://a;0][LDAP://c;0][LDAP://UPPER;0]";

    test_gplink_equality(gplink, correct_gplink_string);
}

void ADMCTestGplink::test_move_up() {
    const QString gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;0][LDAP://UPPER;0]";
    Gplink gplink(gplink_string);

    gplink.move_up("b");

    gplink.move_up("non existing gpo");

    const QString correct_gplink_string = "[LDAP://b;0][LDAP://a;0][LDAP://c;0][LDAP://UPPER;0]";

    test_gplink_equality(gplink, correct_gplink_string);
}

void ADMCTestGplink::test_move_down() {
    const QString gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;0][LDAP://UPPER;0]";
    Gplink gplink(gplink_string);

    gplink.move_down("b");

    gplink.move_down("non existing gpo");

    const QString correct_gplink_string = "[LDAP://a;0][LDAP://c;0][LDAP://b;0][LDAP://UPPER;0]";

    test_gplink_equality(gplink, correct_gplink_string);
}

void ADMCTestGplink::test_get_option() {
    const QString gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;1][LDAP://UPPER;0]";
    Gplink gplink(gplink_string);

    const bool b_option = gplink.get_option("b", GplinkOption_Disabled);
    QVERIFY(b_option == false);

    const bool c_option = gplink.get_option("c", GplinkOption_Disabled);
    QVERIFY(c_option == true);

    // Should return for non-existing gpo
    const bool non_existing_option = gplink.get_option("non_existing_option", GplinkOption_Disabled);
    QVERIFY(non_existing_option == false);
}

void ADMCTestGplink::test_set_option() {
    const QString gplink_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;1][LDAP://UPPER;0]";
    Gplink gplink(gplink_string);

    gplink.set_option("b", GplinkOption_Disabled, true);
    gplink.set_option("c", GplinkOption_Enforced, true);

    const QString correct_gplink_string = "[LDAP://a;0][LDAP://b;1][LDAP://c;3][LDAP://UPPER;0]";

    test_gplink_equality(gplink, correct_gplink_string);
}

void ADMCTestGplink::test_equals() {
    const QString a_string = "[LDAP://a;0][LDAP://b;0][LDAP://c;1][LDAP://UPPER;0]";
    Gplink a_gplink(a_string);
    QVERIFY(a_gplink.equals(a_string));

    const QString b_string = "[LDAP://a;0][LDAP://b;0]";
    Gplink b_gplink(b_string);
    QVERIFY(!b_gplink.equals(a_string));

    // Comparison should be case insensitive
    const QString c_string = "[LDAP://a;0][LDAP://b;0]";
    const QString d_string = "[LDAP://A;0][LDAP://B;0]";
    Gplink c_gplink(c_string);
    QVERIFY(c_gplink.equals(d_string));
}

void test_gplink_equality(const Gplink &a, const QString &b) {
    const QString fail_msg = QString("gplink test failed\nthis = \t  %1\ncorrect = %2").arg(a.to_string(), b);
    QVERIFY2((a.to_string() == b), fail_msg.toUtf8().constData());
}

QTEST_MAIN(ADMCTestGplink)
