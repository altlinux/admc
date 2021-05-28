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

#ifndef FIND_WIDGET_H
#define FIND_WIDGET_H

/**
 * Provides a way for user to find objects. FilterWidget is
 * used for filter input and FindResults for displaying
 * objects. Used by FindObjectDialog and SelectObjectDialog.
 */

#include <QWidget>

class FilterWidget;
class FindResults;
class QStandardItem;
class QPushButton;
class AdObject;
class SelectBaseWidget;
template <typename T> class QList;
template <typename K, typename V> class QHash;

#define FIND_BUTTON_LABEL QT_TRANSLATE_NOOP("FindWidget", "Find")

class FindWidget final : public QWidget {
Q_OBJECT

public:
    FindResults *find_results;
    
    FindWidget(const QList<QString> classes, const QString &default_base);

    // NOTE: returned items need to be re-parented or deleted!
    QList<QList<QStandardItem *>> get_selected_rows() const;

private slots:
    void find();
    void on_thread_finished();
    void handle_find_thread_results(const QHash<QString, AdObject> &results);

private:
    FilterWidget *filter_widget;
    QPushButton *find_button;
    QPushButton *stop_button;
    SelectBaseWidget *select_base_widget;
};

#endif /* FIND_WIDGET_H */
