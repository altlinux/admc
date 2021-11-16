/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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
#include "select_policy_dialog.h"
#include "settings.h"
#include "utils.h"

#include <QDebug>
#include <QFormLayout>
#include <QMenu>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>

enum GplinkColumn {
    GplinkColumn_Name,
    GplinkColumn_Disabled,
    GplinkColumn_Enforced,
    GplinkColumn_COUNT
};

enum GplinkRole {
    GplinkRole_DN = Qt::UserRole + 1,
};

const QSet<GplinkColumn> option_columns = {
    GplinkColumn_Disabled,
    GplinkColumn_Enforced,
};

const QHash<GplinkColumn, GplinkOption> column_to_option = {
    {GplinkColumn_Disabled, GplinkOption_Disabled},
    {GplinkColumn_Enforced, GplinkOption_Enforced},
};

QString gplink_option_to_display_string(const QString &option);

GroupPolicyTab::GroupPolicyTab() {
    ui = new Ui::GroupPolicyTab();
    ui->setupUi(this);

    model = new QStandardItemModel(0, GplinkColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
        {
            {GplinkColumn_Name, tr("Name")},
            {GplinkColumn_Disabled, tr("Disabled")},
            {GplinkColumn_Enforced, tr("Enforced")},
        });

    ui->view->setModel(model);

    new GpoptionsEdit(ui->gpo_options_check, &edits, this);
    edits_connect_to_tab(edits, this);

    settings_restore_header_state(SETTING_group_policy_tab_header_state, ui->view->header());

    enable_widget_on_selection(ui->remove_button, ui->view);

    connect(
        ui->remove_button, &QAbstractButton::clicked,
        this, &GroupPolicyTab::on_remove_button);
    connect(
        ui->add_button, &QAbstractButton::clicked,
        this, &GroupPolicyTab::on_add_button);
    connect(
        ui->view, &QWidget::customContextMenuRequested,
        this, &GroupPolicyTab::on_context_menu);
    connect(
        model, &QStandardItemModel::itemChanged,
        this, &GroupPolicyTab::on_item_changed);
}

GroupPolicyTab::~GroupPolicyTab() {
    settings_save_header_state(SETTING_group_policy_tab_header_state, ui->view->header());

    delete ui;
}

void GroupPolicyTab::load(AdInterface &ad, const AdObject &object) {
    const QString gplink_string = object.get_string(ATTRIBUTE_GPLINK);
    gplink = Gplink(gplink_string);
    original_gplink_string = gplink_string;

    reload_gplink();

    PropertiesTab::load(ad, object);
}

bool GroupPolicyTab::apply(AdInterface &ad, const QString &target) {
    bool total_success = true;

    const bool gplink_changed = !gplink.equals(original_gplink_string);
    if (gplink_changed) {
        const QString gplink_string = gplink.to_string();

        const bool replace_success = ad.attribute_replace_string(target, ATTRIBUTE_GPLINK, gplink_string);

        if (!replace_success) {
            total_success = false;
        }

        if (replace_success) {
            original_gplink_string = gplink_string;
        }
    }

    const bool apply_success = PropertiesTab::apply(ad, target);
    if (!apply_success) {
        total_success = false;
    }

    return total_success;
}

void GroupPolicyTab::on_context_menu(const QPoint pos) {
    const QModelIndex index = ui->view->indexAt(pos);
    const QString gpo = index.data(GplinkRole_DN).toString();
    if (gpo.isEmpty()) {
        return;
    }

    auto menu = new QMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    menu->addAction(tr("Remove link"), [this, gpo]() {
        const QList<QString> removed = {gpo};
        remove_link(removed);
    });
    menu->addAction(tr("Move up"), [this, gpo]() {
        move_link_up(gpo);
    });
    menu->addAction(tr("Move down"), [this, gpo]() {
        move_link_down(gpo);
    });

    menu->popup(pos);
}

void GroupPolicyTab::on_add_button() {
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    auto dialog = new SelectPolicyDialog(ad, this);
    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        [this, dialog]() {
            const QList<QString> selected = dialog->get_selected_dns();

            add_link(selected);
        });
}

void GroupPolicyTab::on_remove_button() {
    const QItemSelectionModel *selection_model = ui->view->selectionModel();
    const QList<QModelIndex> selected_raw = selection_model->selectedRows();

    QList<QString> selected;
    for (auto index : selected_raw) {
        const QString gpo = index.data(GplinkRole_DN).toString();

        selected.append(gpo);
    }

    remove_link(selected);
}

void GroupPolicyTab::add_link(QList<QString> gps) {
    for (auto gp : gps) {
        gplink.add(gp);
    }

    reload_gplink();

    emit edited();
}

void GroupPolicyTab::remove_link(QList<QString> gps) {
    for (auto gp : gps) {
        gplink.remove(gp);
    }

    reload_gplink();

    emit edited();
}

void GroupPolicyTab::move_link_up(const QString &gpo) {
    gplink.move_up(gpo);

    reload_gplink();

    emit edited();
}

void GroupPolicyTab::move_link_down(const QString &gpo) {
    gplink.move_down(gpo);

    reload_gplink();

    emit edited();
}

void GroupPolicyTab::reload_gplink() {
    model->removeRows(0, model->rowCount());

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString base = g_adconfig->domain_dn();
    const SearchScope scope = SearchScope_All;
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QList<QString> attributes = {ATTRIBUTE_DISPLAY_NAME};
    const QHash<QString, AdObject> results_case = ad.search(base, scope, filter, attributes);
    // NOTE: need to convert all dn keys to lowercase
    // because gplink stores them in lower case
    const QHash<QString, AdObject> results = [&]() {
        QHash<QString, AdObject> out;

        for (const QString &key_case : results_case.keys()) {
            const QString key = key_case.toLower();
            const AdObject value = results_case[key_case];
            out[key] = value;
        }

        return out;
    }();

    const QList<QString> gpo_list = gplink.get_gpo_list();

    for (const QString &gpo : gpo_list) {
        // NOTE: Gplink may contain deleted gpo's, in which
        // case the link is "invalid" and there's no real
        // object for the policy
        const bool link_is_valid = results.contains(gpo);

        const QString name = [&]() {
            if (link_is_valid) {
                const AdObject object = results[gpo];
                const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);

                return display_name;
            } else {
                return tr("Not found");
            }
        }();

        const QList<QStandardItem *> row = make_item_row(GplinkColumn_COUNT);
        row[GplinkColumn_Name]->setText(name);
        set_data_for_row(row, gpo, GplinkRole_DN);

        for (const auto column : option_columns) {
            QStandardItem *item = row[column];
            item->setCheckable(true);

            const Qt::CheckState checkstate = [=]() {
                const GplinkOption option = column_to_option[column];
                const bool option_is_set = gplink.get_option(gpo, option);
                if (option_is_set) {
                    return Qt::Checked;
                } else {
                    return Qt::Unchecked;
                }
            }();
            item->setCheckState(checkstate);
        }

        if (!link_is_valid) {
            row[GplinkColumn_Name]->setIcon(QIcon::fromTheme("dialog-error"));

            for (QStandardItem *item : row) {
                item->setToolTip(tr("The GPO for this link could not be found. It maybe have been recently created and is being replicated or it could have been deleted."));
            }
        }

        model->appendRow(row);
    }

    model->sort(GplinkColumn_Name);
}

void GroupPolicyTab::on_item_changed(QStandardItem *item) {
    const GplinkColumn column = (GplinkColumn) item->column();

    if (option_columns.contains(column)) {
        const QModelIndex index = item->index();
        const QString gpo = index.data(GplinkRole_DN).toString();
        const GplinkOption option = column_to_option[column];
        const bool is_checked = (item->checkState() == Qt::Checked);

        gplink.set_option(gpo, option, is_checked);

        reload_gplink();

        emit edited();
    }
}
