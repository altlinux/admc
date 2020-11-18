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

#ifndef RENAME_DIALOG_H
#define RENAME_DIALOG_H

#include <QString>
#include <QDialog>
#include <QList>
#include <QMap>

class AttributeEdit;
class StringEdit;
class QPushButton;

class RenameDialog final : public QDialog {
Q_OBJECT

public:
    RenameDialog(const QString &target_arg);

private slots:
    void accept();
    void update_buttons();
    void reset();

private:
    QString target;
    QList<AttributeEdit *> all_edits;
    StringEdit *name_edit;
    QString old_name_for_message;
    QPushButton *ok_button;
    QPushButton *reset_button;
};

#endif /* RENAME_DIALOG_H */
