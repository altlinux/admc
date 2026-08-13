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

#ifndef CUSTOMIZE_COLUMNS_DIALOG_H
#define CUSTOMIZE_COLUMNS_DIALOG_H

/**
 * Dialog for customizing columns of a view. Presents a list
 * of checkbox which correspond to view's columns. When
 * dialog is accepted, the columns that were unchecked are
 * hidden. Used by console widgets for results views but may
 * be used for other views as well.
 */

#include <QDialog>

class CustomizeColumnsDialogPrivate;
class QTreeView;

class CustomizeColumnsDialog final : public QDialog {
    Q_OBJECT

public:
    CustomizeColumnsDialog(QTreeView *view, const QList<int> &default_columns, QWidget *parent);

public slots:
    void accept() override;

private:
    CustomizeColumnsDialogPrivate *d;
};

#endif /* CUSTOMIZE_COLUMNS_DIALOG_H */
