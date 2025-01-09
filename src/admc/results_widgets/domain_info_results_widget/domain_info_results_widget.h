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

#ifndef DOMAIN_INFO_RESULTS_WIDGET_H
#define DOMAIN_INFO_RESULTS_WIDGET_H

#include <QWidget>

namespace Ui {
class DomainInfoResultsWidget;
}

class ConsoleWidget;
class AdInterface;
class QStandardItemModel;
class AdObject;
class QStandardItem;
class QSpinBox;
template <typename T, typename U>
class QHash;
class QLabel;

struct DomainInfo_SearchResults {
    QList<QStandardItem*> tree_items;
    QString domain_functional_level;
    QString forest_functional_level;
    QString domain_schema_version;
    QString domain_controller_version;
};

class DomainInfoResultsWidget : public QWidget
{
    Q_OBJECT

    enum DomainInfoTreeItemRole {
        DomainInfoTreeItemRole_DN = Qt::UserRole + 1,
        DomainInfoTreeItemRole_ItemType,

        DomainInfoRole_COUNT
    };

    enum DomainInfoTreeItemType {
        DomainInfoTreeItemType_Site,
        DomainInfoTreeItemType_ServersContainer,
        DomainInfoTreeItemType_Host,
        DomainInfoTreeItemType_FSMO_Container,
        DomainInfoTreeItemType_FSMO_Role,

        DomainInfoTreeItemType_COUNT
    };

    enum DomainSchemaVersion {
        DomainSchemaVersion_WindowsServer2008R2 = 47,
        DomainSchemaVersion_WindowsServer2012 = 56,
        DomainSchemaVersion_WindowsServer2012R2 = 69,
        DomainSchemaVersion_WindowsServer2016 = 87,
        DomainSchemaVersion_WindowsServer2019 = 88,
    };

public:
    explicit DomainInfoResultsWidget(ConsoleWidget *console_arg);
    ~DomainInfoResultsWidget();

    void update();

public slots:
    void  update_fsmo_roles(const QString &new_master_dn, const QString &fsmo_role_string);

private:
    Ui::DomainInfoResultsWidget *ui;

    ConsoleWidget *console;
    QStandardItemModel *model;

    void update_defaults();
    QList<QStandardItem*> get_tree_items(AdInterface &ad);
    DomainInfo_SearchResults search_results();
    void populate_widgets(DomainInfo_SearchResults results);
    void add_host_items(QStandardItem *site_item, const AdObject &site_object, AdInterface &ad);
    void set_label_failed(QLabel *label, bool failed);
    QStandardItem *add_tree_item(const QString &text, const QIcon &icon, DomainInfoTreeItemType type, QStandardItem *parent_item = nullptr);

    QString schema_version_to_string(int version);
    QString functionality_level_to_string(int level);
};

#endif // DOMAIN_INFO_RESULTS_WIDGET_H
