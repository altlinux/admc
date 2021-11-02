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

#ifndef EDIT_QUERY_ITEM_WIDGET_H
#define EDIT_QUERY_ITEM_WIDGET_H

/**
 * Widget used for editing queries. Used in edit query
 * dialog and create query dialog.
 */

#include <QWidget>

class FilterDialog;

namespace Ui {
class EditQueryItemWidget;
}

class EditQueryItemWidget : public QWidget {
    Q_OBJECT

public:
    Ui::EditQueryItemWidget *ui;

    EditQueryItemWidget(QWidget *parent = nullptr);
    ~EditQueryItemWidget();

    QString name() const;
    QString description() const;
    QString filter() const;
    QString base() const;
    bool scope_is_children() const;
    QByteArray filter_state() const;

    void clear();
    void set_data(const QString &name, const QString &description, const bool scope_is_children, const QByteArray &filter_state);

private:
    FilterDialog *filter_dialog;

    void update_filter_display();
};

#endif /* EDIT_QUERY_ITEM_WIDGET_H */
