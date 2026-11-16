/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

/**
 * @file edit_query_item.h
 * @brief Widget used for editing queries.
 */

#ifndef EDIT_QUERY_ITEM_WIDGET_H
#define EDIT_QUERY_ITEM_WIDGET_H

#include <QVariant>
#include <QWidget>

namespace Ui {
class EditQueryItemWidget;
}

/**
 * Widget used for editing queries. Used in edit query
 * dialog and create query dialog.
 */
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

    void set_data(const QString &name, const QString &description, const bool scope_is_children, const QByteArray &filter_state, const QString &filter);

private:
    QVariant filter_dialog_state;

    void open_filter_dialog();
};

#endif /* EDIT_QUERY_ITEM_WIDGET_H */
