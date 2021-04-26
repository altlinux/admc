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

#ifndef STRING_MULTI_EDIT_H
#define STRING_MULTI_EDIT_H

#include "multi_edits/attribute_multi_edit.h"

class QLineEdit;
class QCheckBox;

/**
 * Edit for editing string attributes of multiple objects.
 */

class StringMultiEdit : public AttributeMultiEdit {
Q_OBJECT
public:
    StringMultiEdit(const QString &attribute_arg, QList<AttributeMultiEdit *> &edits_out, QObject *parent);

    void add_to_layout(QFormLayout *layout) override;
    bool apply(AdInterface &ad, const QList<QString> &target_list) override;
    void reset() override;

private slots:
    void on_check_toggled();

private:
    QCheckBox *check;
    QWidget *check_and_label_wrapper;
    QLineEdit *edit;
    QString attribute;
};

#endif /* STRING_MULTI_EDIT_H */
