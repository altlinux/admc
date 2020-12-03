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

#ifndef OCTET_MULTI_EDITOR_H
#define OCTET_MULTI_EDITOR_H

#include "editors/attribute_editor.h"
#include "editors/octet_editor.h"

class QListWidget;
class QPushButton;
class QComboBox;
class QListWidgetItem;

class OctetMultiEditor final : public AttributeEditor {
Q_OBJECT

public:
    OctetMultiEditor(const QString attribute_arg, const QList<QByteArray> values, QWidget *parent);

    QList<QByteArray> get_new_values() const;

private slots:
    void on_format_combo();
    void edit_item(QListWidgetItem *item);
    void on_add();
    void on_remove();

private:
    QString attribute;
    QListWidget *list_widget;
    QPushButton *add_button;
    QComboBox *format_combo;

    void add_value(const QByteArray value);
};

#endif /* OCTET_MULTI_EDITOR_H */
