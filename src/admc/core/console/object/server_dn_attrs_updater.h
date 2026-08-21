/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
 * Copyright (C) 2026 Semyon Knyazev
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

#ifndef SERVERDNATTRSUPDATER_H
#define SERVERDNATTRSUPDATER_H

#include <QString>

class AdInterface;
class QByteArray;

class ServerDnAttrsUpdater {
public:
    ServerDnAttrsUpdater(const QString &server_dn);

    void update_for_delete(AdInterface &ad);
    void update_for_move(AdInterface &ad, const QString &new_server_dn);

private:
    void update_replica_locations(AdInterface &ad, const QString &new_server_dn);
    void update_fsmo_role_owner(AdInterface &ad, const QString &new_server_dn);
    void update_inter_site_topology_generator(AdInterface &ad, const QString &new_server_dn);

    void cleanup_missing_site_dn_values(QList<QByteArray> &server_setts_dn_list, const QStringList &actual_site_list);

    QString dn;
};

#endif // SERVERDNATTRSUPDATER_H
