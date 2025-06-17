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

#include "inherited_policies_widget.h"
#include "ui_inherited_policies_widget.h"
#include "utils.h"
#include "settings.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_impl.h"
#include "console_widget/console_widget.h"
#include "console_impls/item_type.h"
#include "gplink.h"
#include "managers/icon_manager.h"
#include "managers/gplink_manager.h"
#include "globals.h"
#include "ad_object.h"
#include "ad_utils.h"

#include <QStandardItemModel>
#include <QStringList>


InheritedPoliciesWidget::InheritedPoliciesWidget(ConsoleWidget *console_arg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InheritedPoliciesWidget),
    console(console_arg) {
    ui->setupUi(this);

    model = new QStandardItemModel(0, InheritedPoliciesColumns_COUNT, this);
    set_horizontal_header_labels_from_map(model,
            {
                {InheritedPoliciesColumns_Prority, tr("Priority")},
                {InheritedPoliciesColumns_Name, tr("Name")},
                {InheritedPoliciesColumns_Location, tr("Location")},
            });
    ui->view->set_model(model);
    const QVariant state = settings_get_variant(SETTING_inheritance_widget_state);
    ui->view->restore_state(state, {
                                InheritedPoliciesColumns_Prority,
                                InheritedPoliciesColumns_Name,
                                InheritedPoliciesColumns_Location,
                            });
    ui->view->set_drag_drop_enabled(false);
}

InheritedPoliciesWidget::~InheritedPoliciesWidget() {
    const QVariant state = ui->view->save_state();
    settings_set_variant(SETTING_inheritance_widget_state, state);

    delete ui;
}

void InheritedPoliciesWidget::update(const QModelIndex &ou_index) {
    selected_scope_index = ou_index;
    ou_dn = ou_index.data(PolicyOURole_DN).toString();
    update(ou_dn);
}

void InheritedPoliciesWidget::update(const QString &ou_dn_arg) {
    show_busy_indicator();
    ou_dn = ou_dn_arg;
    AdInterface ad;
    if (!ad.is_connected()) {
        return;
    }

    model->removeRows(0, model->rowCount());
    add_enabled_policy_items(ad, ou_dn_arg);
    remove_link_duplicates();
    set_priority_to_items();
    model->sort(InheritedPoliciesColumns_Prority);
    hide_busy_indicator();
}

void InheritedPoliciesWidget::hide_not_enforced_inherited_links(bool hide) {
    const Gplink gplink = Gplink(g_gplink_manager->ou_gplink(ou_dn));
    const QStringList gplink_strings = gplink.get_gpo_list();
    for (int row = 0; row < model->rowCount(); ++row) {
        if (!gplink_strings.contains(model->item(row)->data(RowRole_DN).toString()) &&
                !model->item(row)->data(RowRole_IsEnforced).toBool()) {
            ui->view->set_row_hidden(row, hide);
        }
    }
}

void InheritedPoliciesWidget::add_enabled_policy_items(AdInterface &ad, const QString ou_dn, bool inheritance_blocked) {
    const AdObject ou_obj = ad.search_object(ou_dn, {ATTRIBUTE_NAME,
                                                     ATTRIBUTE_GPOPTIONS,
                                                     ATTRIBUTE_GPLINK});
    if (ou_obj.is_empty()) {
        return;
    }

    const Gplink gplink = Gplink(ou_obj.get_string(ATTRIBUTE_GPLINK));
    const QStringList enforced_links = gplink.enforced_gpo_dn_list();
    const QStringList disabled_links = gplink.disabled_gpo_dn_list();
    int enforced_policy_row = 0;
    for (QString gpo_dn : gplink.get_gpo_list()) {
        if (disabled_links.contains(gpo_dn)) {
            continue;
        }

        bool policy_is_enforced = enforced_links.contains(gpo_dn);
        QList<QStandardItem *> row;

        if (policy_is_enforced) {
            row = make_item_row(InheritedPoliciesColumns_COUNT);
            load_item(row, ad, ou_obj.get_string(ATTRIBUTE_NAME), gpo_dn, policy_is_enforced);
            model->insertRow(enforced_policy_row, row);
            ++enforced_policy_row;
        }
        else if (!inheritance_blocked) {
            row = make_item_row(InheritedPoliciesColumns_COUNT);
            load_item(row, ad, ou_obj.get_string(ATTRIBUTE_NAME), gpo_dn, policy_is_enforced);
            model->appendRow(row);
        }
    }


    bool inheritance_is_blocked = ou_obj.get_string(ATTRIBUTE_GPOPTIONS) == GPOPTIONS_BLOCK_INHERITANCE || inheritance_blocked;
    add_enabled_policy_items(ad, dn_get_parent(ou_dn), inheritance_is_blocked);
}

void InheritedPoliciesWidget::remove_link_duplicates() {
    QStringList dn_list;
    for (int row = 0; row < model->rowCount(); ++row) {
        // The same dn is set to each item in the row
        const QString dn = model->index(row, 0).data(RowRole_DN).toString();
        if (dn_list.contains(dn)) {
            model->removeRow(row);
            --row;
            continue;
        }
        dn_list << dn;
    }
}

void InheritedPoliciesWidget::set_priority_to_items() {
    for(int row = 0; row < model->rowCount(); ++row) {
        model->item(row, InheritedPoliciesColumns_Prority)->
                setData(row + 1, Qt::DisplayRole);
    }
}

void InheritedPoliciesWidget::load_item(const QList<QStandardItem *> row, AdInterface &ad, const QString &ou_name, const QString &policy_dn, bool is_enforced) {
    AdObject gpo = ad.search_object(policy_dn, {ATTRIBUTE_DISPLAY_NAME});
    if (gpo.is_empty()) {
        return;
    }

    set_data_for_row(row, policy_dn, RowRole_DN);
    set_data_for_row(row, is_enforced, RowRole_IsEnforced);

    const QString name = gpo.get_string(ATTRIBUTE_DISPLAY_NAME);
    row[InheritedPoliciesColumns_Name]->setText(name);
    row[InheritedPoliciesColumns_Location]->setText(ou_name);
    if (is_enforced)
        row[0]->setIcon(g_icon_manager->item_icon(ItemIcon_Policy_Enforced));
    else
        row[0]->setIcon(g_icon_manager->item_icon(ItemIcon_Policy_Link));
}
