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

#ifndef CUSTOMIZE_COLUMNS_DIALOG_H
#define CUSTOMIZE_COLUMNS_DIALOG_H

/**
 * Dialog for customizing columns of a results view in
 * console widget. Presents a list of checkbox which
 * correspond to view's columns. When dialog is accepted,
 * the columns that were unchecked are hidden.
 */

#include <QDialog>

#include "results_description.h"

class QTreeView;
class QCheckBox;

class CustomizeColumnsDialog final : public QDialog {
Q_OBJECT

public:
    CustomizeColumnsDialog(const ResultsDescription &results, QWidget *parent);

public slots:
    void accept() override;
    void restore_defaults();

private:
    ResultsDescription results;
    QList<QCheckBox *> checkbox_list;
};

#endif /* CUSTOMIZE_COLUMNS_DIALOG_H */
