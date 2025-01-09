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

#ifndef SECURITY_SORT_WARNING_DIALOG
#define SECURITY_SORT_WARNING_DIALOG

/**
 * Dialog that warns about incorrect ACL sort order and
 * offers to fix it. Opens when switching to security
 * tab and incorrect order is detected.
 */

#include <QDialog>

namespace Ui {
class SecuritySortWarningDialog;
}

class SecuritySortWarningDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::SecuritySortWarningDialog *ui;

    SecuritySortWarningDialog(QWidget *parent);
    ~SecuritySortWarningDialog();
};

#endif /* SECURITY_SORT_WARNING_DIALOG */
