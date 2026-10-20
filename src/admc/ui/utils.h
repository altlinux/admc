/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
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

#ifndef UTILS_H
#define UTILS_H

#include "adldap.h"
#include "core/ad.h"
#include "core/search_thread.h"

class AdInterface;
class ConsoleWidget;
class QAbstractItemView;
class QPersistentModelIndex;
class QStandardItem;
class QString;
class QVariant;
class QWidget;
template <typename T> class QList;

#define debug_print(a, args...) \
    printf("%s(%s:%d) " a, __func__, __FILE__, __LINE__, ##args)
#define trace(a, args...) debug_print(a "\n", ##args)

void enable_widget_on_selection(QWidget *widget, QAbstractItemView *view);

void show_busy_indicator();
void hide_busy_indicator();

bool ad_connected(const AdInterface &ad, QWidget *parent);
bool ad_failed(const AdInterface &ad, QWidget *parent);

QList<QString> get_selected_dn_list(ConsoleWidget *console, const int type,
                                    const int dn_role);
QString get_selected_target_dn(ConsoleWidget *console,
                               const int type,
                               const int dn_role);

void center_widget(QWidget *widget);

bool string_contains_bad_chars(const QString &string,
                               const QString &bad_chars);

bool verify_object_name(const QString &name,
                        QWidget *parent);

void search_thread_display_errors(SearchThread *thread, QWidget *parent);

#endif /* UTILS_H */
