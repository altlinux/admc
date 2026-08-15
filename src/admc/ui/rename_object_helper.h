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

#ifndef RENAME_OBJECT_HELPER_H
#define RENAME_OBJECT_HELPER_H

#include <QDialogButtonBox>
#include <QObject>
#include <QPushButton>

class QLineEdit;
class QDialog;
class AttributeEdit;
class AdInterface;

class RenameObjectHelper : public QObject {
    Q_OBJECT

public:
    RenameObjectHelper(AdInterface &ad, const QString &target_arg, QLineEdit *name_edit_arg, const QList<AttributeEdit *> &edits_arg, QDialog *parent_dialog, QList<QLineEdit *> required = QList<QLineEdit *>(), QDialogButtonBox *button_box = nullptr);

    static void success_msg(const QString &old_name);
    static void fail_msg(const QString &old_name);

    bool accept() const;
    QString get_new_name() const;
    QString get_new_dn() const;

private slots:
    void on_edited();

private:
    QString target;
    QLineEdit *name_edit;
    QList<AttributeEdit *> edits;
    QDialog *parent_dialog;
    QList<QLineEdit *> required_list;
    QPushButton *ok_button;
};

#endif /* RENAME_OBJECT_HELPER_H */
