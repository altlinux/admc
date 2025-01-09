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

#ifndef DOMAININFOIMPL_H
#define DOMAININFOIMPL_H

/**
 * Impl for root domain info item in the scope tree
 */

#include "console_widget/console_impl.h"

class ConsoleWidget;
class QModelIndex;
template <typename T>
class QList;
class DomainInfoResultsWidget;
class AdInterface;

class DomainInfoImpl : public ConsoleImpl {
    Q_OBJECT

public:
    explicit DomainInfoImpl(ConsoleWidget *console_arg);

    void selected_as_scope(const QModelIndex &index) override;
    void refresh(const QList<QModelIndex> &index_list) override;

    virtual QList<QAction *> get_all_custom_actions() const override;
    QSet<QAction *> get_custom_actions(const QModelIndex &index, const bool single_selection) const override;
    QSet<StandardAction> get_standard_actions(const QModelIndex &index, const bool single_selection) const override;

    QList<QString> column_labels() const override;

    void load_domain_info_item(const AdInterface &ad);

private:
    QAction *edit_fsmo_action;
    QAction *connection_options_action;

    DomainInfoResultsWidget *domain_info_results_widget;

    void open_fsmo_dialog();
    void open_connection_options();
};


#endif // DOMAININFOIMPL_H
