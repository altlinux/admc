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

#ifndef GPLINK_H
#define GPLINK_H

#include <QHash>
#include <QList>
#include <QString>

enum GplinkOption {
    GplinkOption_None,
    GplinkOption_Disable,
    GplinkOption_Enforce,
    GplinkOption_COUNT
};

class Gplink {
public:
    Gplink();
    Gplink(const QString &gplink_string);

    QString to_string() const;
    QList<QString> get_gpos() const;
    GplinkOption get_option(const QString &gpo) const;

    void add(const QString &gpo);
    void remove(const QString &gpo);
    void move_up(const QString &gpo);
    void move_down(const QString &gpo);
    void set_option(const QString &gpo, const GplinkOption option);

private:
    QList<QString> gpos_in_order;
    QHash<QString, GplinkOption> options;
};

GplinkOption gplink_option_from_string(const QString &option_string);
QString gplink_option_to_string(const GplinkOption option);
QString gplink_option_to_display_string(const GplinkOption option);

#endif /* GPLINK_H */
