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

#include "find_dialog.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "settings.h"
#include "containers_widget.h"
#include "utils.h"
#include "filter.h"
#include "filter_widget/filter_widget.h"

#include <QString>
#include <QList>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QLabel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QPushButton>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QDebug>

enum FindDialogColumn {
    FindDialogColumn_Name,
    FindDialogColumn_DN,
    FindDialogColumn_COUNT
};

FindDialog::FindDialog()
: QDialog()
{
    setWindowTitle(tr("Find dialog"));
    resize(600, 600);
    setAttribute(Qt::WA_DeleteOnClose);

    model = new QStandardItemModel(0, FindDialogColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model, {
        {FindDialogColumn_Name, tr("Name")},
        {FindDialogColumn_DN, tr("DN")}
    });

    auto view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSortingEnabled(true);
    view->setSelectionMode(QAbstractItemView::ContiguousSelection);
    view->setModel(model);

    filter_widget = new FilterWidget();

    const auto layout = new QVBoxLayout(this);
    layout->addWidget(filter_widget);
    layout->addWidget(view);

    setup_column_toggle_menu(view, model, {FindDialogColumn_Name});

    // TODO: sort after each new search
    // model->sort(FindDialogColumn_Name);
    // TODO: column sizing
    // for (int col = 0; col < view->model()->columnCount(); col++) {
    //     view->resizeColumnToContents(col);
    // }

    load("");

    // const auto list = ADCONFIG()->get_possible_attributes({"top", "person", "organizationalPerson", "user"});
    // qInfo() << list;
    
    // const auto only_strings =
    // [list]() {
    //     QList<QString> out;
    //     for (auto attribute : list) {
    //         const AttributeType type = ADCONFIG()->get_attribute_type(attribute);
    //         if (type == AttributeType_Unicode) {
    //             out.append(attribute);
    //         }
    //     }
    //     return out;
    // }();
    // qInfo() << only_strings;
}

void FindDialog::load(const QString &filter) {
    QList<QString> attributes = {
        ATTRIBUTE_NAME,
        ATTRIBUTE_OBJECT_CLASS,
        ATTRIBUTE_DESCRIPTION
    };
    attributes.append(ATTRIBUTE_DISTINGUISHED_NAME);
    const QList<QString> extra_columns = ADCONFIG()->get_extra_columns();
    attributes.append(extra_columns);

    const QHash<QString, AdObject> search_results = AD()->search(filter, attributes, SearchScope_All);

    model->removeRows(0, model->rowCount());
    for (const AdObject &object : search_results.values()) {
        const QString dn = object.get_dn();
        const QString name = dn_get_rdn(dn);

        const QList<QStandardItem *> row = make_item_row(FindDialogColumn_COUNT);
        row[FindDialogColumn_Name]->setText(name);
        row[FindDialogColumn_DN]->setText(dn);

        model->appendRow(row);
    }
}
