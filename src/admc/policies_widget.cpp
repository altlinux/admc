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
#include "ad_interface.h"
#include "details_dialog.h"
#include "rename_dialog.h"
#include "utils.h"
#include "filter.h"

#include <QTreeView>
#include <QLabel>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>
#include <QProcess>

enum PoliciesColumn {
    PoliciesColumn_Name,
    PoliciesColumn_DN,
    PoliciesColumn_COUNT,
};

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
        {PoliciesColumn_DN, tr("DN")}
    });

    view->setModel(model);

    setup_column_toggle_menu(view, model, {PoliciesColumn_Name});

    const auto label = new QLabel(tr("Policies"), this);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(label);
    layout->addWidget(view);

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &PoliciesWidget::on_context_menu);

    show_only_in_dev_mode(this);

    connect(
        AD(), &AdInterface::modified,
        this, &PoliciesWidget::reload);
    reload();
}

void PoliciesWidget::reload() {
    model->removeRows(0, model->rowCount());

    const QList<QString> search_attributes = {ATTRIBUTE_DISPLAY_NAME};
    const QString filter = filter_EQUALS(ATTRIBUTE_OBJECT_CLASS, CLASS_GP_CONTAINER);
    const QHash<QString, AdObject> search_results = AD()->search(filter, search_attributes, SearchScope_All);

    for (auto dn : search_results.keys()) {
        const AdObject object  = search_results[dn];
        const QString display_name = object.get_string(ATTRIBUTE_DISPLAY_NAME);
        
        const QList<QStandardItem *> row = make_item_row(PoliciesColumn_COUNT);
        row[PoliciesColumn_Name]->setText(display_name);
        row[PoliciesColumn_DN]->setText(dn);

        model->appendRow(row);
    }

    model->sort(PoliciesColumn_Name);
}

void PoliciesWidget::on_context_menu(const QPoint pos) {
    const QString dn = get_dn_from_pos(pos, view, PoliciesColumn_DN);
    if (dn.isEmpty()) {
        return;
    }

    const AdObject object = AD()->search_object(dn);

    QMenu menu;
    menu.addAction(tr("Details"), [this, dn]() {
        DetailsDialog::open_for_target(dn);
    });
    menu.addAction(tr("Edit Policy"), [this, dn, object]() {
        edit_policy(dn, object);
    });
    menu.addAction(tr("Rename"), [this, dn]() {
        auto rename_dialog = new RenameDialog(dn);
        rename_dialog->open();
    });
    menu.addAction(tr("Delete"), [dn]() {
        AD()->delete_gpo(dn);
    });

    exec_menu_from_view(&menu, view, pos);
}

void PoliciesWidget::edit_policy(const QString &dn, const AdObject &object) {
    // Start policy edit process
    const auto process = new QProcess();

    const QString sysvol_path = object.get_string(ATTRIBUTE_GPC_FILE_SYS_PATH);
    const QString smb_path = sysvol_path_to_smb(sysvol_path);

    const QString program_name = "/home/kevl/admc/build/gpgui";

    QStringList args = {"-p", smb_path};

    qint64 pid;
    const bool start_success = process->startDetached(program_name, args, QString(), &pid);

    printf("edit_policy\n");
    printf("smb_path=%s\n", qPrintable(smb_path));
    printf("pid=%lld\n", pid);
    printf("start_success=%d\n", start_success);
}
