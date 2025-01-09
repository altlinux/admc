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

#ifndef PROPERTIES_MULTI_DIALOG_H
#define PROPERTIES_MULTI_DIALOG_H

/**
 * Properties dialog for editing attributes of multiple
 * objects at the same time. Only editing, attribute values
 * aren't displayed. For most objects the only attribute is
 * description. Only users have many tabs/attributes.
 */

#include <QDialog>

class AttributeEdit;
class AdInterface;
class QCheckBox;

namespace Ui {
class PropertiesMultiDialog;
}

class PropertiesMultiDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::PropertiesMultiDialog *ui;

    PropertiesMultiDialog(AdInterface &ad, const QList<QString> &target_list_arg, const QList<QString> &class_list);
    ~PropertiesMultiDialog();

signals:
    void applied();

private slots:
    void accept() override;
    void on_edited();

private:
    QList<QString> target_list;
    QList<AttributeEdit *> edit_list;
    QHash<AttributeEdit *, QCheckBox *> check_map;
    QPushButton *apply_button;

    bool apply();
};

#endif /* PROPERTIES_MULTI_DIALOG_H */
