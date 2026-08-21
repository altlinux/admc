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

#ifndef SITEDNATTRSUPDATER_H
#define SITEDNATTRSUPDATER_H

#include <QString>

class AdInterface;

class SiteDnAttrsUpdater {
public:
    explicit SiteDnAttrsUpdater(const QString &site_dn);

    void update_for_delete(AdInterface &ad);
    void update_for_rename(AdInterface &ad, const QString &new_dn);

private:
    QString dn;

    void update_site_links(AdInterface &ad, const QString &new_site_dn);
    void update_dns_records(AdInterface &ad, const QString &new_site_dn);
};

#endif // SITEDNATTRSUPDATER_H
