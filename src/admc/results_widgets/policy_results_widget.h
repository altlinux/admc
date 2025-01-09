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

#ifndef POLICY_RESULTS_WIDGET_H
#define POLICY_RESULTS_WIDGET_H

/**
 * Displays OU's linked to currently selected policy.
 */

#include <QWidget>
#include "gplink.h"

class QStandardItemModel;
class QStandardItem;
class QMenu;
class ResultsView;
class ADMCTestPolicyResultsWidget;

namespace Ui {
class PolicyResultsWidget;
}

class PolicyResultsWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::PolicyResultsWidget *ui;

    PolicyResultsWidget(QWidget *parent);
    ~PolicyResultsWidget();

    // Loads links for this policy. Nothing is done if given
    // index is not a policy.
    void update(const QModelIndex &index);

    void update(const QString &gpo);

    ResultsView *get_view() const;

    QString get_current_gpo() const;

signals:
    void ou_gplink_changed(const QString &ou_dn, const Gplink &gplink, const QString &policy_dn, GplinkOption option = GplinkOption_NoOption);

private:
    QStandardItemModel *model;
    QString gpo;
    QMenu *context_menu;

    void on_item_changed(QStandardItem *item);
    void open_context_menu(const QPoint &pos);
    void delete_link();

    friend class ADMCTestPolicyResultsWidget;
};

#endif /* POLICY_RESULTS_WIDGET_H */
