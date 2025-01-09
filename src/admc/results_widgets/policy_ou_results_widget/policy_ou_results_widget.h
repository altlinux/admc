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

#ifndef POLICY_OU_RESULTS_WIDGET_H
#define POLICY_OU_RESULTS_WIDGET_H

/**
 * Displays OU's linked to currently selected policy.
 */

#include <QWidget>

class ConsoleWidget;
class LinkedPoliciesWidget;
class InheritedPoliciesWidget;
//class ADMCTestPolicyOUResultsWidget;

namespace Ui {
class PolicyOUResultsWidget;
}

class PolicyOUResultsWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::PolicyOUResultsWidget *ui;

    PolicyOUResultsWidget(ConsoleWidget *console);
    ~PolicyOUResultsWidget();

    // Loads links for given OU. Nothing is done if given
    // index is not an OU in policy tree.
    void update(const QModelIndex &index);
    void update_inheritance_widget(const QModelIndex &index);
    void update_links_widget(const QModelIndex &index);

private:
    ConsoleWidget *console;
    LinkedPoliciesWidget *links_widget;
    InheritedPoliciesWidget *inheritance_widget;

    // friend ADMCTestPolicyOUResultsWidget;
};

#endif /* POLICY_OU_RESULTS_WIDGET_H */
