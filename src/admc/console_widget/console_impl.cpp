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

#include "console_widget/console_impl.h"

#include "console_widget/console_widget.h"
#include <QSet>

ConsoleImpl::ConsoleImpl(ConsoleWidget *console_arg)
: QObject(console_arg) {
    console = console_arg;
}

void ConsoleImpl::fetch(const QModelIndex &index) {

}

bool ConsoleImpl::can_drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {
    return false;
}

void ConsoleImpl::drop(const QList<QPersistentModelIndex> &dropped_list, const QSet<int> &dropped_type_list, const QPersistentModelIndex &target, const int target_type) {

}

QString ConsoleImpl::get_description(const QModelIndex &index) const {
    return QString();
}

void ConsoleImpl::activate(const QModelIndex &index) {

}

QList<QAction *> ConsoleImpl::get_all_custom_actions() const {
    return {};
}

QSet<QAction *> ConsoleImpl::get_custom_actions(const QModelIndex &index, const bool single_selection) const {
    return {};
}

QSet<QAction *> ConsoleImpl::get_disabled_custom_actions(const QModelIndex &index, const bool single_selection) const {
    return {};
}

QSet<StandardAction> ConsoleImpl::get_standard_actions(const QModelIndex &index, const bool single_selection) const {
    return {};
}

QSet<StandardAction> ConsoleImpl::get_disabled_standard_actions(const QModelIndex &index, const bool single_selection) const {
    return {};
}

void ConsoleImpl::copy(const QList<QModelIndex> &index_list) {

}

void ConsoleImpl::cut(const QList<QModelIndex> &index_list) {

}
    
void ConsoleImpl::rename(const QList<QModelIndex> &index_list) {

}
    
void ConsoleImpl::delete_action(const QList<QModelIndex> &index_list) {

}
    
void ConsoleImpl::paste(const QList<QModelIndex> &index_list) {

}
    
void ConsoleImpl::print(const QList<QModelIndex> &index_list) {

}
    
void ConsoleImpl::refresh(const QList<QModelIndex> &index_list) {

}
    
void ConsoleImpl::properties(const QList<QModelIndex> &index_list) {

}