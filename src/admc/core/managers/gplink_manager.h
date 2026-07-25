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

#ifndef GPLINKMANAGER_H
#define GPLINKMANAGER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QMutex>

class GPLinkManager final : public QObject {
    Q_OBJECT

public:
    explicit GPLinkManager(QObject *parent = nullptr);

    void update();
    void set_gplink(const QString &ou_dn, const QString &gplink_str);
    QString ou_gplink(const QString &ou_dn) const;
    bool update_failed();
    const QHash<QString, QString>& gplinks_map() const;
    QStringList linked_ou_list(const QString &policy_dn) const;

private:
    // OU DN - key, GPLink string - value. DN keys can contain domain dn.
    QHash<QString, QString> ou_links;
    mutable QMutex mutex;
    bool is_updated;
    bool failed_to_update = false;
};

#endif // GPLINKMANAGER_H
