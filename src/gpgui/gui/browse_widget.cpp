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

#include "browse_widget.h"
#include "pol_editor.h"
#include "xml_editor.h"
#include "file.h"
#include "utils.h"

#include <QTreeView>
#include <QString>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QLabel>

enum BrowseColumn {
    BrowseColumn_Name,
    BrowseColumn_Path,
    BrowseColumn_COUNT
};

#define POL_EXTENSION ".pol"
#define XML_EXTENSION ".xml"

BrowseWidget::BrowseWidget()
: QWidget()
{   
    model = new QStandardItemModel(0, BrowseColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {BrowseColumn_Name, tr("Name")},
        {BrowseColumn_Path, tr("Path")}
    });

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

    view->setModel(model);

    setup_column_toggle_menu(view, model, {BrowseColumn_Name});

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(new QLabel(tr("policy_path=") + policy_path));

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &BrowseWidget::on_context_menu);
}

void BrowseWidget::change_target(const QString &policy_path_arg) {
    policy_path = policy_path_arg;

    model->clear();

    add_entry_recursively(policy_path, nullptr);
    
    // TODO: use proxy for this
    view->setColumnHidden(BrowseColumn_Path, true);
}

void BrowseWidget::add_entry_recursively(const QString &path, QStandardItem *parent) {
    // TODO: ok to use system f-n basename for smp url?
    const QByteArray path_array = path.toLatin1();
    const char *path_cstr = path_array.constData();
    const QString name = basename(path_cstr);

    const QList<QStandardItem *> row = make_item_row(BrowseColumn_COUNT);
    row[BrowseColumn_Name]->setText(name);
    row[BrowseColumn_Path]->setText(path);

    if (parent == nullptr) {
        model->appendRow(row);
    } else {
        parent->appendRow(row);
    }

    const QList<QString> children = file_get_children(path);
    for (auto child : children) {
        add_entry_recursively(child, row[0]);
    }
}

void BrowseWidget::on_context_menu(const QPoint pos) {
    const QModelIndex base_index = view->indexAt(pos);
    if (!base_index.isValid()) {
        return;
    }
    const QModelIndex path_index = base_index.siblingAtColumn(BrowseColumn_Path);
    const QString path = path_index.data().toString();

    const QPoint global_pos = view->mapToGlobal(pos);


    const bool is_pol = path.endsWith(POL_EXTENSION);
    const bool is_xml = path.endsWith(XML_EXTENSION);

    if (is_pol || is_xml) {
        QMenu menu(this);

        QAction *edit_action = menu.addAction(tr("Edit"));
        connect(
            edit_action, &QAction::triggered,
            [this, path, is_pol, is_xml]() {
                if (is_pol) {
                    auto pol_editor = new PolEditor(path);
                    pol_editor->open();
                } else if (is_xml) {
                    auto xml_editor = new XmlEditor(path);
                    xml_editor->open();
                }
            });

        menu.exec(global_pos);
    }
}
