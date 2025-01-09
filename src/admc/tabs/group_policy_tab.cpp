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

#include "tabs/group_policy_tab.h"
#include "tabs/ui_group_policy_tab.h"

#include "adldap.h"
#include "attribute_edits/gpoptions_edit.h"
#include "globals.h"
#include "select_dialogs/select_policy_dialog.h"
#include "settings.h"
#include "utils.h"
#include "console_widget/console_widget.h"
#include "results_widgets/policy_ou_results_widget/policy_ou_results_widget.h"
#include "results_widgets/policy_ou_results_widget/inherited_policies_widget.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_ou_impl.h"

#include <QFormLayout>
#include <QMenu>
#include <QPushButton>
#include <QStandardItemModel>
#include <QCheckBox>

GroupPolicyTab::GroupPolicyTab(QList<AttributeEdit *> *edit_list, ConsoleWidget *console_widget, const QString &ou_dn, QWidget *parent)
: QWidget(parent), console(console_widget) {
    ui = new Ui::GroupPolicyTab();
    ui->setupUi(this);

    inheritance_widget = new InheritedPoliciesWidget(console, this);

    gpo_options_check = new QCheckBox(tr("Block policy inheritance"), this);
    auto options_edit = new GpoptionsEdit(gpo_options_check, this);

    ui->verticalLayout->addWidget(inheritance_widget);
    ui->verticalLayout->addWidget(gpo_options_check);

    edit_list->append({options_edit});

    target_ou_index = search_gpo_ou_index(console, ou_dn);

    if (console && target_ou_index.isValid()) {

        inheritance_widget->update(target_ou_index);
        connect(gpo_options_check, &QCheckBox::toggled, [this](bool toggled) {
            inheritance_widget->hide_not_enforced_inherited_links(toggled);
            });
        connect(options_edit, &GpoptionsEdit::gp_options_changed, [this](bool inheritance_blocked) {
            console->get_item(target_ou_index)->setData(inheritance_blocked, PolicyOURole_Inheritance_Block);
            inheritance_widget->update(target_ou_index);

            PolicyOUResultsWidget *result_ou_widget = dynamic_cast<PolicyOUResultsWidget*>(console->get_result_widget_for_index(target_ou_index));
            if (result_ou_widget)
                result_ou_widget->update_inheritance_widget(target_ou_index);
            });
    }
}

GroupPolicyTab::~GroupPolicyTab() {

    delete ui;
}
