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

#ifndef EDIT_QUERY_ITEM_WIDGET_H
#define EDIT_QUERY_ITEM_WIDGET_H

/**
 * Widget used for editing queries. Used in edit query
 * dialog and create query dialog.
 */

#include <QWidget>
#include <QString>
#include <QModelIndex>

class QLineEdit;
class QTextEdit;
class FilterWidget;
class SearchBaseWidget;

class EditQueryItemWidget : public QWidget {
Q_OBJECT

public:
    EditQueryItemWidget();

    void load(const QModelIndex &index);
    void save(QString &name, QString &description, QString &filter, QString &search_base, QByteArray &filter_state) const;

private:
    QLineEdit *name_edit;
    QLineEdit *description_edit;
    QTextEdit *filter_display;
    FilterWidget *filter_widget;
    SearchBaseWidget *search_base_widget;

    void update_filter_display();
};

#endif /* EDIT_QUERY_ITEM_WIDGET_H */
