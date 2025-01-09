/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef MANAGER_WIDGET_H
#define MANAGER_WIDGET_H

#include <QWidget>

class AdObject;
class AdInterface;

namespace Ui {
class ManagerWidget;
}

class ManagerWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::ManagerWidget *ui;

    ManagerWidget(QWidget *parent = nullptr);
    ~ManagerWidget();

    void set_attribute(const QString &attribute);
    void load(const AdObject &object);
    bool apply(AdInterface &ad, const QString &dn) const;

    QString get_manager() const;
    void reset();

signals:
    void edited();

private slots:
    void on_change();
    void on_properties();
    void on_clear();

private:
    QString manager_attribute;
    QString current_value;

    void load_value(const QString &value);
};

#endif /* MANAGER_WIDGET_H */
