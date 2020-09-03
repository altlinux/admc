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

// #include <libsmbclient.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

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
    model->setHorizontalHeaderItem(BrowseColumn_Name, new QStandardItem("Name"));
    model->setHorizontalHeaderItem(BrowseColumn_Path, new QStandardItem("DN"));

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setContextMenuPolicy(Qt::CustomContextMenu);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);
    view->setModel(model);

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addWidget(new QLabel("policy_path=" + policy_path));

    QObject::connect(
        view, &QWidget::customContextMenuRequested,
        this, &BrowseWidget::on_context_menu);
}

void BrowseWidget::change_policy_path(const QString &policy_path_arg) {
    policy_path = policy_path_arg;

    model->clear();

    add_entry_recursively(policy_path, nullptr);
    
    // TODO: use proxy for this
    view->setColumnHidden(BrowseColumn_Path, true);
}

void BrowseWidget::add_entry_recursively(const QString &path, QStandardItem *parent) {
    const QByteArray path_array = path.toLatin1();
    const char *path_cstr = path_array.constData();

    QList<QStandardItem *> row;
    for (int i = 0; i < BrowseColumn_COUNT; i++) {
        row.append(new QStandardItem());
    }

    const QString name = basename(path_cstr);
    row[BrowseColumn_Name]->setText(name);
    row[BrowseColumn_Path]->setText(path);

    if (parent == nullptr) {
        model->appendRow(row);
    } else {
        parent->appendRow(row);
    }

    struct stat filestat;
    stat(path_cstr, &filestat);

    const bool is_dir = S_ISDIR(filestat.st_mode);
    if (is_dir) {
        DIR *dirp = opendir(path_cstr);

        struct dirent *entry;
        while ((entry = readdir(dirp)) != NULL) {
            const QString entry_name(entry->d_name);
            
            const bool is_dot_path = (entry_name == "." || entry_name == "..");
            if (is_dot_path) {
                continue;
            }

            const QString entry_path = path + "/" + entry_name;
            add_entry_recursively(entry_path, row[0]);
        }

        closedir(dirp);
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
