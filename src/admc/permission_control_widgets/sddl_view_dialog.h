/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2025-2026 BaseALT Ltd.
 * Copyright (C) 2025 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#ifndef SDDL_VIEW_DIALOG_H
#define SDDL_VIEW_DIALOG_H

#include <QDialog>

namespace Ui {
class SDDLViewDialog;
}

struct security_descriptor;

class SDDLViewDialog : public QDialog {
    Q_OBJECT

public:
    explicit SDDLViewDialog(QWidget *parent = nullptr);
    ~SDDLViewDialog();

public slots:
    void update(security_descriptor *sd_arg);
    void set_trustee(const QByteArray &trustee_arg);

private:
    Ui::SDDLViewDialog *ui;
    security_descriptor *sd;
    QByteArray trustee;

    QString get_sddl() const;
    void update();
};

#endif // SDDL_VIEW_DIALOG_H
