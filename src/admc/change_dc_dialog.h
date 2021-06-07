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

class QStandardItem;
class QListWidget;
class QCheckBox;

class ChangeDCDialog : public QDialog {
Q_OBJECT

public:
    ChangeDCDialog(QStandardItem *domain_head_item, QWidget *parent);

    void accept() override;

private:
    QListWidget *list_widget;
    QStandardItem *domain_head_item;
    QCheckBox *save_dc_checkbox;
};

#endif /* CHANGE_DC_DIALOG_H */
