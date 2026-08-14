/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
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

#include "ui/widget/result/base.h"

class PSOEditWidget;

class PSOResultsWidget final : public ResultsWidgetBase {
    Q_OBJECT

public:
    explicit PSOResultsWidget(QWidget *parent = nullptr);
    virtual ~PSOResultsWidget() = default;

    virtual void update(const QModelIndex &index) override;
    virtual void update(const AdObject &pso) override;

private:
    PSOEditWidget *pso_edit_widget;

    virtual void on_apply() override;
    virtual void on_edit() override;
    virtual void on_cancel_edit() override;
    virtual void set_editable(bool is_editable) override;

    virtual QStringList changed_attrs() override;
};

#endif // PSO_RESULTS_WIDGET_H
