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

#ifndef CREATE_POLICY_DIALOG_H
#define CREATE_POLICY_DIALOG_H

/**
 * Creates a GPO.
 */

#include <QDialog>
#include <QList>
#include <QString>

class QLineEdit;
class ConsoleWidget;

class CreatePolicyDialog : public QDialog {
    Q_OBJECT

public:
    CreatePolicyDialog(ConsoleWidget *console_arg);

public slots:
    void open() override;
    void accept() override;

private:
    ConsoleWidget *console;
    QLineEdit *name_edit;
};

#endif /* CREATE_POLICY_DIALOG_H */
