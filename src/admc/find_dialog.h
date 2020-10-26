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

#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H

/**
 * User inputs a filter through FilterWidget. Then the
 * dialog displays search results from that filter. TODO:
 * update comment depending on if end up displaying results
 * in real time or not.
 */

#include <QDialog>

class QStandardItemModel;
class FilterWidget;

class FindDialog final : public QDialog {
Q_OBJECT

public:
    FindDialog();

private slots:
    void on_filter_changed();

private:
    QStandardItemModel *model;
    FilterWidget *filter_widget;

    void load(const QString &filter);
};

#endif /* FIND_DIALOG_H */
