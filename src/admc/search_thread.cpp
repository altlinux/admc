/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "search_thread.h"

#include "adldap.h"
#include "settings.h"
#include "status.h"
#include "utils.h"

#include <QHash>

SearchThread::SearchThread(const QString base_arg, const SearchScope scope_arg, const QString &filter_arg, const QList<QString> attributes_arg) :
stop_flag(false), base(base_arg), scope(scope_arg), filter(filter_arg), attributes(attributes_arg), id(0), m_failed_to_connect(false),
m_hit_object_display_limit(false) {

    static int id_max = 0;
    id = id_max;
    id_max++;
}

void SearchThread::stop() {
    stop_flag = true;
}

void SearchThread::run() {
    AdInterface ad;
    if (!ad.is_connected()) {
        m_failed_to_connect = true;

        return;
    }

    AdCookie cookie;

    const int object_display_limit = settings_get_variant(SETTING_object_display_limit).toInt();

    int total_results_count = 0;

    while (true) {
        QHash<QString, AdObject> results;

        const bool success = ad.search_paged(base, scope, filter, attributes, &results, &cookie);

        total_results_count += results.count();

        if (total_results_count > object_display_limit) {
            m_hit_object_display_limit = true;

            break;
        }

        ad_messages = ad.messages();

        emit results_ready(results);

        const bool search_interrupted = (!success || stop_flag);
        if (search_interrupted) {
            break;
        }

        if (!cookie.more_pages()) {
            break;
        }
    }
}

int SearchThread::get_id() const {
    return id;
}

bool SearchThread::failed_to_connect() const {
    return m_failed_to_connect;
}

bool SearchThread::hit_object_display_limit() const {
    return m_hit_object_display_limit;
}

QList<AdMessage> SearchThread::get_ad_messages() const {
    return ad_messages;
}

void search_thread_display_errors(SearchThread *thread, QWidget *parent) {
    if (thread->failed_to_connect()) {
        error_log({QCoreApplication::translate("object_impl.cpp", "Failed to connect to server while searching for objects.")}, parent);
    } else if (thread->hit_object_display_limit()) {
        error_log({QCoreApplication::translate("object_impl.cpp", "Could not load all objects. Increase object display limit in Filter Options or reduce number of objects by applying a filter. Filter Options is accessible from main window's menubar via the \"View\" menu.")}, parent);
    }
}
