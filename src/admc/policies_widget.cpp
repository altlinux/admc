/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "policies_widget.h"
#include "adldap.h"
#include "properties_dialog.h"
#include "rename_policy_dialog.h"
#include "globals.h"
#include "utils.h"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>
#include <QProcess>

enum PoliciesColumn {
    PoliciesColumn_Name,
    PoliciesColumn_COUNT,
};

enum PoliciesRole {
    PoliciesRole_DN = Qt::UserRole + 1,
};

// TODO: respond to AD object signals

PoliciesWidget::PoliciesWidget()
: QWidget()
{   
    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setSortingEnabled(true);

    model = new QStandardItemModel(0, PoliciesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {PoliciesColumn_Name, tr("Name")},
    });

    view->setModel(model);

    const auto label = new QLabel(tr("Policies"), this);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(label);
    layout->addWidget(view);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &PoliciesWidget::on_context_menu);

    show_only_in_dev_mode(this);

    // TODO: inline if won't use this with ad signals
    reload();
}

void PoliciesWidget::reload() {
    model->removeRows(0, model->rowCount());

    const QList<QString> search_attributes = {ATTRIBUTE_DISPLAY_NAME};
    const QString filter = filter_CONDITION(Condition_Equals, ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QHash<QString, AdObject> search_results = ad.search(filter, search_attributes, SearchScope_All);

    for (auto dn : search_results.keys()) {
        const AdObject object  = search_results[dn];
        const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
        
        const QList<QStandardItem *> row = make_item_row(PoliciesColumn_COUNT);
        row[PoliciesColumn_Name]->setText(display_name);

        set_data_for_row(row, dn, PoliciesRole_DN);

        model->appendRow(row);
    }

    model->sort(PoliciesColumn_Name);
}

void PoliciesWidget::on_context_menu(const QPoint pos) {
    const QModelIndex index = view->indexAt(pos);
    const QString dn = index.data(PoliciesRole_DN).toString();
    if (dn.isEmpty()) {
        return;
    }

    
    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const AdObject object = ad.search_object(dn);

    QMenu menu;
    menu.addAction(PropertiesDialog::display_name(), [dn]() {
        PropertiesDialog::open_for_target(dn);
    });
    menu.addAction(tr("Edit Policy"), [this, object]() {
        edit_policy(object);
    });
    menu.addAction(tr("Rename"), [this, dn]() {
        auto dialog = new RenamePolicyDialog(dn, this);
        dialog->open();
    });
    menu.addAction(tr("Delete"), [dn]() {
        AdInterface ad_delete;
        if (ad_failed(ad_delete)) {
            return;
        }
        ad_delete.delete_gpo(dn);
    });

    exec_menu_from_view(&menu, view, pos);
}

void PoliciesWidget::edit_policy(const AdObject &object) {
    // Start policy edit process
    const auto process = new QProcess();

    AdInterface ad;
    if (ad_failed(ad)) {
        return;
    }

    const QString sysvol_path = object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);
    const QString smb_path = ad.sysvol_path_to_smb(sysvol_path);

    const QString program_name = "/home/kevl/admc/build/gpgui";

    QStringList args = {"-p", smb_path};

    qint64 pid;
    const bool start_success = process->startDetached(program_name, args, QString(), &pid);

    printf("edit_policy\n");
    printf("smb_path=%s\n", qPrintable(smb_path));
    printf("pid=%lld\n", pid);
    printf("start_success=%d\n", start_success);
}
