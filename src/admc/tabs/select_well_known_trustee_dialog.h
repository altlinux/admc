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

#ifndef SELECT_WELL_KNOWN_TRUSTEE_DIALOG_H
#define SELECT_WELL_KNOWN_TRUSTEE_DIALOG_H

/**
 * Dialog for selecting well known trustee's. Well known
 * trustee's are represented by unique SID's and aren't
 * related to any real objects, so need a custom dialog for
 * this.
 */

#include <QDialog>

namespace Ui {
class SelectWellKnownTrusteeDialog;
}

class SelectWellKnownTrusteeDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::SelectWellKnownTrusteeDialog *ui;

    SelectWellKnownTrusteeDialog(QWidget *parent);
    ~SelectWellKnownTrusteeDialog();

    QList<QByteArray> get_selected() const;
};

#endif /* SELECT_WELL_KNOWN_TRUSTEE_DIALOG_H */
