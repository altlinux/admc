/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#ifndef CREATE_OBJECT_DIALOG_H
#define CREATE_OBJECT_DIALOG_H

/**
 * Base class for dialogs that create objects. 
 */

#include <QDialog>

#include "widget_state.h"

class QLineEdit;
class QDialogButtonBox;
class AttributeEdit;

class CreateObjectDialog : public QDialog {
    Q_OBJECT

public:
    using QDialog::QDialog;

    static void success_msg(const QString &old_name);
    static void fail_msg(const QString &old_name);

    // Required widgets will be use to determine when dialog
    // can be accepted. Widget list will be used to
    // save/restore their state.
    void init(QLineEdit *name_edit_arg, QDialogButtonBox *button_box, const QList<AttributeEdit *> &edits_list, const QList<QLineEdit *> &required_list, const QList<QWidget *> &widget_list, const QString &object_class);

    QString get_created_dn() const;
    void set_parent_dn(const QString &dn);
    void open() override;
    void accept() override;
    void on_edited();

private:
    QString parent_dn;
    QLineEdit *name_edit;
    QList<AttributeEdit *> m_edit_list;
    QList<QLineEdit *> m_required_list;
    QPushButton *ok_button;
    QString m_object_class;
    WidgetState m_state;

    QString get_created_name() const;
};

#endif /* CREATE_OBJECT_DIALOG_H */
