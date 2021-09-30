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

#ifndef RENAME_OBJECT_DIALOG_H
#define RENAME_OBJECT_DIALOG_H

#include <QDialog>

class AttributeEdit;
class QLineEdit;
class QPushButton;

namespace Ui {
    class RenameObjectDialog;
}

class RenameObjectDialog final : public QDialog {
    Q_OBJECT

public:
    RenameObjectDialog(const QString &target_arg, QWidget *parent);

    static void success_msg(const QString &old_name);
    static void fail_msg(const QString &old_name);

    QString get_new_dn() const;

public slots:
    void accept();

private slots:
    void on_edited();
    void reset();

private:
    Ui::RenameObjectDialog *ui;
    QString target;
    QList<AttributeEdit *> all_edits;
    QPushButton *ok_button;
    QPushButton *reset_button;
};

#endif /* RENAME_OBJECT_DIALOG_H */
