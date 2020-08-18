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

#include "attribute_widgets.h"
#include "attribute_display_strings.h"

#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QMessageBox>

void make_attribute_edits(const QList<QString> attributes, QGridLayout *layout, QMap<QString, QLineEdit *> *edits_out) {
    for (auto attribute : attributes) {
        const QString attribute_display_string = get_attribute_display_string(attribute);
        const auto label = new QLabel(attribute_display_string);

        auto edit = new QLineEdit();
        edits_out->insert(attribute, edit);

        const int row = layout->rowCount();
        layout->addWidget(label, row, 0);
        layout->addWidget(edit, row, 1);
    }
}

QList<AdResult> apply_attribute_edits(const QMap<QString, QLineEdit *> &edits, const QString &dn) {
    QList<AdResult> results;

    for (auto attribute : edits.keys()) {
        const QLineEdit *edit = edits[attribute];
        const QString new_value = edit->text();
        const QString old_value = AdInterface::instance()->attribute_get(dn, attribute);

        if (new_value != old_value) {
            const AdResult result = AdInterface::instance()->attribute_replace(dn, attribute, new_value);

            results.append(result);
        }
    }

    return results;
}

void show_warnings_for_error_results(const QList<AdResult> &results, QWidget *parent) {
    for (auto result : results) {
        if (!result.success) {
            QMessageBox::critical(parent, QObject::tr("Error"), result.error_with_context);
        }
    }
}

bool no_errors(const QList<AdResult> &results) {
    for (auto result : results) {
        if (!result.success) {
            return false;
        }
    }

    return true;
}
