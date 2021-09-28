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

#ifndef FIND_WIDGET_H
#define FIND_WIDGET_H

/**
 * Provides a way for user to find objects. FilterWidget is
 * used for filter input and FindResults for displaying
 * objects. Used by FindObjectDialog and SelectObjectDialog.
 */

#include <QWidget>

class QStandardItem;
class AdObject;
class QMenu;

namespace Ui {
    class FindWidget;
}

class FindWidget final : public QWidget {
    Q_OBJECT

public:
    FindWidget(QWidget *parent = nullptr);
    ~FindWidget();

    void init(const QList<QString> classes, const QString &default_base);

    void add_actions(QMenu *action_menu, QMenu *view_menu);

    void clear();

    // NOTE: returned items need to be re-parented or deleted!
    QList<QString> get_selected_dns() const;

private slots:
    void find();
    void handle_find_thread_results(const QHash<QString, AdObject> &results);

private:
    Ui::FindWidget *ui;
    QStandardItem *head_item;

    void on_thread_finished();
};

#endif /* FIND_WIDGET_H */
