/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "admc_test_group_type_edit.h"

#include "attribute_edits/group_type_edit.h"

#include <QComboBox>
#include <QFormLayout>

void ADMCTestGroupTypeEdit::init() {
    ADMCTest::init();

    combo = new QComboBox(parent_widget);

    edit = new GroupTypeEdit(combo, parent_widget);

    const QString name = TEST_GROUP;
    dn = test_object_dn(name, CLASS_GROUP);
    const bool create_success = ad.object_add(dn, CLASS_GROUP);
    QVERIFY(create_success);
}

void ADMCTestGroupTypeEdit::edited_signal() {
    bool edited_signal_emitted = false;
    connect(
        edit, &AttributeEdit::edited,
        this,
        [&edited_signal_emitted]() {
            edited_signal_emitted = true;
        });

    combo->setCurrentIndex(1);
    QVERIFY(edited_signal_emitted);
}

void ADMCTestGroupTypeEdit::load() {
    const AdObject object = ad.search_object(dn);
    edit->load(ad, object);

    QCOMPARE(combo->currentIndex(), 0);
}

void ADMCTestGroupTypeEdit::apply_unmodified() {
    test_edit_apply_unmodified(edit, dn);
}

void ADMCTestGroupTypeEdit::apply() {
    load();

    combo->setCurrentIndex(1);
    const bool apply_success = edit->apply(ad, dn);
    QVERIFY(apply_success);

    const AdObject updated_object = ad.search_object(dn);
    const GroupType type = updated_object.get_group_type();
    QCOMPARE(type, GroupType_Distribution);
}

QTEST_MAIN(ADMCTestGroupTypeEdit)
