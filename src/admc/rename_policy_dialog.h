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

#ifndef RENAME_DIALOG_POLICY_H
#define RENAME_DIALOG_POLICY_H

/**
 * Special rename dialog for policy objects. Policy objects
 * are never renamed because their names are GUID's. Instead
 * their display name attribute is modified.
 */

#include <QDialog>
#include <QString>

class QLineEdit;
class QPushButton;

class RenamePolicyDialog final : public QDialog {
Q_OBJECT

public:
    RenamePolicyDialog(const QString &target_arg, QWidget *parent);

private slots:
    void accept();
    void on_edited();
    void reset();

private:
    QString target;
    QLineEdit *name_edit;
    QPushButton *ok_button;
    QPushButton *reset_button;
};

#endif /* RENAME_DIALOG_POLICY_H */
