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

#ifndef CREATE_QUERY_DIALOG_H
#define CREATE_QUERY_DIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QModelIndex>

class QLineEdit;
class FilterWidget;
class SearchBaseWidget;

class CreateQueryDialog : public QDialog {
Q_OBJECT

public:
    CreateQueryDialog(const QModelIndex &parent_index_arg, QWidget *parent);

    QString get_name() const;
    QString get_description() const;
    QString get_filter() const;
    QString get_search_base() const;

private:
    QLineEdit *name_edit;
    QLineEdit *description_edit;
    FilterWidget *filter_widget;
    SearchBaseWidget *search_base_widget;
    QList<QString> sibling_names;
    QModelIndex parent_index;

    void accept() override;
};

#endif /* CREATE_QUERY_DIALOG_H */
