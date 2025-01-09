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

#ifndef RENAME_POLICY_DIALOG_H
#define RENAME_POLICY_DIALOG_H

/**
 * Special rename dialog for policy objects. Policy objects
 * are never renamed because their names are GUID's. Instead
 * their display name attribute is modified.
 */

#include <QDialog>

class QPushButton;
class AdInterface;

namespace Ui {
class RenamePolicyDialog;
}

class RenamePolicyDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::RenamePolicyDialog *ui;

    RenamePolicyDialog(AdInterface &ad, const QString &target_dn, QWidget *parent);
    ~RenamePolicyDialog();

    void accept() override;

private:
    QString target_dn;
    QString target_name;

private slots:
    void on_edited();
};

#endif /* RENAME_DIALOG_POLICY_H */
