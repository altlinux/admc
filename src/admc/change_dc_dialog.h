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

#ifndef CHANGE_DC_DIALOG_H
#define CHANGE_DC_DIALOG_H

/**
 * Dialog that allows user to change the domain controller.
 */

#include <QDialog>

namespace Ui {
    class ChangeDCDialog;
}

class ConsoleWidget;

class ChangeDCDialog : public QDialog {
    Q_OBJECT

public:
    ChangeDCDialog(ConsoleWidget *console);

    void open() override;
    void accept() override;
    void reject() override;

private:
    Ui::ChangeDCDialog *ui;
    ConsoleWidget *console;
    QHash<QWidget *, QVariant> original_state;
    
    QList<QWidget *> get_widget_list() const;
};

#endif /* CHANGE_DC_DIALOG_H */
