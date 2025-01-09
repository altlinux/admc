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

#include "console_widget/console_impl.h"

#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"

#include <QSet>
#include <QVariant>

ConsoleImpl::ConsoleImpl(ConsoleWidget *console_arg)
: QObject(console_arg) {
    console = console_arg;
    results_widget = nullptr;
    results_view = nullptr;
}

void ConsoleImpl::fetch(const QModelIndex &index) {
    UNUSED_ARG(index);
}

bool ConsoleImpl::can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(dropped_list);
    UNUSED_ARG(dropped_type_list);
    UNUSED_ARG(target);
    UNUSED_ARG(target_type);

    return false;
}

void ConsoleImpl::drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    UNUSED_ARG(dropped_list);
    UNUSED_ARG(dropped_type_list);
    UNUSED_ARG(target);
    UNUSED_ARG(target_type);
}

QString ConsoleImpl::get_description(const QModelIndex &index) const {
    UNUSED_ARG(index);

    return QString();
}

void ConsoleImpl::activate(const QModelIndex &index) {
    UNUSED_ARG(index);
}

void ConsoleImpl::selected_as_scope(const QModelIndex &index) {
    UNUSED_ARG(index);
}

QList<QAction *> ConsoleImpl::get_all_custom_actions() const {
    return {};
}

QSet<QAction *> ConsoleImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);
    UNUSED_ARG(single_selection);

    return {};
}

QSet<QAction *> ConsoleImpl::get_disabled_custom_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);
    UNUSED_ARG(single_selection);

    return {};
}

QSet<StandardAction> ConsoleImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);
    UNUSED_ARG(single_selection);

    return {};
}

QSet<StandardAction> ConsoleImpl::get_disabled_standard_actions(const QModelIndex &index, const bool single_selection) const {
    UNUSED_ARG(index);
    UNUSED_ARG(single_selection);

    return {};
}

void ConsoleImpl::copy(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);
}

void ConsoleImpl::cut(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);
}

void ConsoleImpl::rename(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);
}

void ConsoleImpl::delete_action(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);
}

void ConsoleImpl::paste(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);
}

void ConsoleImpl::print(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);
}

void ConsoleImpl::refresh(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);
}

void ConsoleImpl::properties(const QList<QModelIndex> &index_list) {
    UNUSED_ARG(index_list);
}

QList<QString> ConsoleImpl::column_labels() const {
    return QList<QString>();
}

QList<int> ConsoleImpl::default_columns() const {
    return QList<int>();
}

void ConsoleImpl::update_results_widget(const QModelIndex &index) const
{
   UNUSED_ARG(index);
}

QVariant ConsoleImpl::save_state() const {
    if (view() != nullptr) {
        return view()->save_state();
    } else {
        return QVariant();
    }
}

void ConsoleImpl::restore_state(const QVariant &state) {
    if (view() != nullptr) {
        view()->restore_state(state, default_columns());
    }
}

QWidget *ConsoleImpl::widget() const {
    if (results_widget != nullptr) {
        return results_widget;
    } else if (results_view != nullptr) {
        return results_view;
    } else {
        return nullptr;
    }
}

ResultsView *ConsoleImpl::view() const {
    return results_view;
}

void ConsoleImpl::set_results_view(ResultsView *results_view_arg) {
    results_view = results_view_arg;
}

void ConsoleImpl::set_results_widget(QWidget *results_widget_arg) {
    results_widget = results_widget_arg;
}
