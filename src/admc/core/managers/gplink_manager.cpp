/*
 * ADMC - AD Management Center
 *
 * This file contains procedures for attribute handling and validation.
 *
 * Copyright (C) 2025-2026 BaseALT Ltd.
 * Copyright (C) 2025 Semyon Knyazev
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

#include "gplink_manager.h"

#include "core/search_thread.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_config.h"
#include "ad_filter.h"
#include "core/globals.h"
#include "gplink.h"
#include "utils.h"

GPLinkManager::GPLinkManager(QObject *parent) : QObject(parent), is_updated(true) {
}

void GPLinkManager::update() {
    if (!is_updated) {
        return;
    }

    is_updated = false;

    const QString &filter = filter_OR({filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_OU),
                                      filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_DOMAIN)});
    auto search_thread = new SearchThread(g_adconfig->domain_dn(),
                                          SearchScope_All,
                                          filter,
                                          {ATTRIBUTE_GPLINK});

    connect(search_thread, &SearchThread::results_ready, this, [this](const QHash<QString, AdObject> &results) {
        for (const QString &dn : results.keys()) {
            // Make gplink trimmed because some values can contain only spaces for some reason
            const QString gplink_str = results[dn].get_string(ATTRIBUTE_GPLINK).trimmed();
            if (!gplink_str.isEmpty()) {
                set_gplink(dn, results[dn].get_string(ATTRIBUTE_GPLINK));
            }
        }
    });

    connect(search_thread, &SearchThread::finished, this, [this, search_thread]() {
        failed_to_update = search_thread->failed_to_connect();
        is_updated = true;

        search_thread->deleteLater();
    });

    search_thread->start();
}

void GPLinkManager::set_gplink(const QString &ou_dn, const QString &gplink_str) {
    QMutexLocker locker(&mutex);
    ou_links[ou_dn] = gplink_str;
}

QString GPLinkManager::ou_gplink(const QString &ou_dn) const {
    return ou_links.value(ou_dn, QString());
}

bool GPLinkManager::update_failed() {
    return failed_to_update;
}

const QHash<QString, QString> &GPLinkManager::gplinks_map() const {
    return ou_links;
}

QStringList GPLinkManager::linked_ou_list(const QString &policy_dn) const {
    QStringList ou_dn_list;
    for (const QString &ou_dn : ou_links.keys()) {
        Gplink gplink = Gplink(ou_links[ou_dn]);
        if (gplink.contains(policy_dn)) {
            ou_dn_list.append(ou_dn);
        }
    }

    return ou_dn_list;
}
