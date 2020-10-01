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

#include "edits/attribute_edit.h"
#include "utils.h"
#include "details_tab.h"

#include "ad_interface.h"

#include <QGridLayout>
#include <QLabel>

void connect_changed_marker(AttributeEdit *edit, QLabel *label) {
    QObject::connect(edit, &AttributeEdit::edited,
        [=]() {
            if (edit->read_only == EditReadOnly_Yes) {
                return;
            }

            const QString current_text = label->text();
            const QString new_text = set_changed_marker(current_text, edit->changed());
            label->setText(new_text);
        });
}

void layout_attribute_edits(QList<AttributeEdit *> edits, QGridLayout *layout) {
    for (auto edit : edits) {
        edit->add_to_layout(layout);
    }
}

bool any_edits_changed(QList<AttributeEdit *> edits) {
    for (auto edit : edits) {
        if (edit->changed()) {
            return true;
        }
    }

    return false;
}

bool verify_attribute_edits(QList<AttributeEdit *> edits, QWidget *parent) {
    bool success = true;

    for (auto edit : edits) {
        const bool verify_success = edit->verify_input(parent);

        if (!verify_success) {
            success = false;
        }
    }

    return success;
}

bool apply_attribute_edits(QList<AttributeEdit *> edits, const QString &dn, QObject *parent, const ApplyIfNotChanged apply_if_not_changed) {
    bool success = true;

    for (auto edit : edits) {
        if (edit->changed() || (apply_if_not_changed == ApplyIfNotChanged_Yes)) {
            const bool apply_success = edit->apply(dn);
            if (!apply_success) {
                success = false;
            }
        }
    }

    return success;
}

void load_attribute_edits(QList<AttributeEdit *> edits, const QString &dn) {
    for (auto edit : edits) {
        edit->load(dn);
    }
}

void connect_edits_to_tab(QList<AttributeEdit *> edits, DetailsTab *tab) {
    for (auto edit : edits) {
        QObject::connect(
            edit, &AttributeEdit::edited,
            tab, &DetailsTab::on_edit_edited);
    }
}

void AttributeEdit::set_read_only(EditReadOnly read_only_arg) {
    read_only = read_only_arg;
}
