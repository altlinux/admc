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

#include "search_thread.h"

#include "adldap.h"
#include "utils.h"

#include <QHash>

SearchThread::SearchThread(const QString base_arg, const SearchScope scope_arg, const QString &filter_arg, const QList<QString> attributes_arg) {
    stop_flag = false;
    base = base_arg;
    scope = scope_arg;
    filter = filter_arg;
    attributes = attributes_arg;

    connect(
        this, &SearchThread::finished,
        this, &QObject::deleteLater);
}

void SearchThread::stop() {
    stop_flag = true;
}

void SearchThread::run() {
    // TODO: handle search/connect failure
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    AdCookie cookie;

    while (true) {
        QHash<QString, AdObject> results;
        
        const bool success = ad.search_paged(base, scope, filter, attributes, &results, &cookie);

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
