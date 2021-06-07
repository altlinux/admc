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

#ifndef EXPIRY_WIDGET_H
#define EXPIRY_WIDGET_H

#include <QFrame>

class QCheckBox;
class QDateEdit;
class AdInterface;
class AdObject;

class ExpiryWidget final : public QFrame {
Q_OBJECT
public:
    ExpiryWidget();

    void load(const AdObject &object);
    void set_read_only(const bool read_only);
    bool apply(AdInterface &ad, const QString &dn) const;

signals:
    void edited();

private slots:
    void on_never_check();
    void on_end_of_check();

private:
    QCheckBox *never_check;
    QCheckBox *end_of_check;
    QDateEdit *edit;

    QString get_new_value() const;
};

#endif /* EXPIRY_WIDGET_H */
