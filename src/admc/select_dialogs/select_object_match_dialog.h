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

#ifndef SELECT_OBJECT_MATCH_DIALOG_H
#define SELECT_OBJECT_MATCH_DIALOG_H

/**
 * Dialog for selecting from multiple matches in select
 * object dialog.
 */

#include <QDialog>

class AdObject;
class QStandardItemModel;

namespace Ui {
class SelectObjectMatchDialog;
}

class SelectObjectMatchDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::SelectObjectMatchDialog *ui;

    SelectObjectMatchDialog(const QHash<QString, AdObject> &search_results, QWidget *parent);
    ~SelectObjectMatchDialog();

    QList<QString> get_selected() const;

private:
    QStandardItemModel *model;
};

#endif /* SELECT_OBJECT_MATCH_DIALOG_H */
