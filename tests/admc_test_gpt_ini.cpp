/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 Korney Gedert
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

#include "admc_test_gpt_ini.h"

#include "ad_utils.h"

#include <QTest>

void ADMCTestGptIni::get_version_data() {
    QTest::addColumn<QString>("ini_contents");
    QTest::addColumn<int>("expected_version");

    QTest::newRow("admc") << "[General]\r\nVersion=131079\r\n" << 131079;
    QTest::newRow("new policy") << "[General]\r\nVersion=0\r\n" << 0;
    QTest::newRow("gpui lf")
        << "[General]\nDisplayName=Policy from GPUI\nVersion=131079\n"
        << 131079;
    QTest::newRow("gpui crlf")
        << "[General]\r\nDisplayName=Policy from GPUI\r\nVersion=131079\r\n"
        << 131079;
    QTest::newRow("version first")
        << "[General]\nVersion=131079\nDisplayName=Policy from GPUI\n"
        << 131079;
    QTest::newRow("no final newline")
        << "[General]\nDisplayName=Policy\nVersion=131079" << 131079;
    QTest::newRow("whitespace and comments")
        << "; GPT.INI\n [General] \r\n\n;Version=1\n Version = 131079 \n"
        << 131079;
    QTest::newRow("other section before general")
        << "[Other]\nVersion=1\n[General]\nVersion=131079\n"
        << 131079;
    QTest::newRow("other section after general")
        << "[General]\nVersion=131079\n[Other]\nVersion=1\n"
        << 131079;
    QTest::newRow("similar key")
        << "[General]\nOtherVersion=1\nVersionExtra=2\nVersion=131079\n"
        << 131079;
    QTest::newRow("version in display name")
        << "[General]\nDisplayName=Version=1\nVersion=131079\n"
        << 131079;
    QTest::newRow("leading zeroes") << "[General]\nVersion=00010\n" << 10;
    QTest::newRow("max signed version") << "[General]\nVersion=2147483647\n"
                                        << 2147483647;

    QTest::newRow("empty") << "" << -1;
    QTest::newRow("no section") << "Version=131079\n" << -1;
    QTest::newRow("wrong section") << "[Other]\nVersion=131079\n" << -1;
    QTest::newRow("missing version") << "[General]\nDisplayName=Policy\n" << -1;
    QTest::newRow("version outside general")
        << "[General]\nDisplayName=Policy\n[Other]\nVersion=131079\n"
        << -1;
    QTest::newRow("only version in display name")
        << "[General]\nDisplayName=Version=131079\n"
        << -1;
    QTest::newRow("empty version") << "[General]\nVersion=\n" << -1;
    QTest::newRow("invalid version") << "[General]\nVersion=invalid\n" << -1;
    QTest::newRow("negative version") << "[General]\nVersion=-1\n" << -1;
    QTest::newRow("overflow") << "[General]\nVersion=2147483648\n" << -1;
    QTest::newRow("trailing garbage") << "[General]\nVersion=131079invalid\n"
                                      << -1;
}

void ADMCTestGptIni::get_version() {
    QFETCH(QString, ini_contents);
    QFETCH(int, expected_version);

    QCOMPARE(gpt_ini_get_version(ini_contents), expected_version);
}

QTEST_GUILESS_MAIN(ADMCTestGptIni)
