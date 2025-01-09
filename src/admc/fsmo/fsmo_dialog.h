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

#ifndef FSMO_DIALOG_H
#define FSMO_DIALOG_H

/**
 * Dialog for viewing and editing FSMO roles.
 */

#include <QDialog>


class AdInterface;

namespace Ui {
class FSMODialog;
}

class FSMODialog : public QDialog {
    Q_OBJECT

public:
    Ui::FSMODialog *ui;

    FSMODialog(AdInterface &ad, QWidget *parent);
    ~FSMODialog();

private slots:
    void gpo_edit_PDC_check_toggled(bool is_checked);

signals:
    void master_changed(const QString &new_master_dn, const QString &string_fsmo_role);
};

#endif /* FSMO_DIALOG_H */
