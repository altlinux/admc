/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#ifndef PSO_RESULTS_WIDGET_H
#define PSO_RESULTS_WIDGET_H

#include <QWidget>

#include "ad_object.h"

namespace Ui {
class PSOResultsWidget;
}

class PSOResultsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PSOResultsWidget(QWidget *parent = nullptr);
    ~PSOResultsWidget();

    void update(const QModelIndex &index);
    void update(const AdObject &pso);

private:
    Ui::PSOResultsWidget *ui;
    AdObject saved_pso_object;

    void on_apply();
    void on_edit();
    void on_cancel();
    QStringList changed_setting_attrs();
    void set_editable(bool is_editable);
};

#endif // PSO_RESULTS_WIDGET_H
