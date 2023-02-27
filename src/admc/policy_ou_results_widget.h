/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "gplink.h"

class QStandardItemModel;
class QStandardItem;
class QMenu;
class ResultsView;
class ConsoleWidget;
class ADMCTestPolicyOUResultsWidget;

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
    void update(const QString &dn);

    ResultsView *get_view() const;

private:
    ConsoleWidget *console;
    QStandardItemModel *model;
    Gplink gplink;
    QString ou_dn;
    QMenu *context_menu;

    void on_item_changed(QStandardItem *item);
    void open_context_menu(const QPoint &pos);
    void remove_link();
    void move_up();
    void move_down();
    void reload_gplink();
    void modify_gplink(void (*modify_function)(Gplink &, const QString &));
    void change_policy_icon(const QString &policy_dn, bool is_checked, GplinkOption option);
    void update_gpo_lists_data(const QString &policy_dn, bool is_checked, GplinkOption option);

    friend ADMCTestPolicyOUResultsWidget;
};

#endif /* POLICY_OU_RESULTS_WIDGET_H */
