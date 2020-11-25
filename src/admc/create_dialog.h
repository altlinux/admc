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

#ifndef CREATE_DIALOG_H
#define CREATE_DIALOG_H

/**
 * Creates an object by letting user fill in attributes. All
 * objects require a name. Depending on object's class,
 * different additional attributes may be set after
 * creation.
 */

#include <QDialog>
#include <QString>
#include <QList>

class AttributeEdit;
class QLineEdit;
class QPushButton;
class PasswordEdit;

class CreateDialog : public QDialog {
Q_OBJECT

public:
    CreateDialog(const QString &parent_dn_arg, const QString &object_class_arg, QWidget *parent);

private slots:
    void accept();
    void on_edited();

private:
    QString parent_dn;
    QString object_class;
    QLineEdit *name_edit;
    QPushButton *create_button;
    QList<AttributeEdit *> all_edits;
    QList<AttributeEdit *> required_edits;
    PasswordEdit *pass_edit;
};

#endif /* CREATE_DIALOG_H */
