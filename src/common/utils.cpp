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

#include "utils.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QAbstractItemView>
#include <QModelIndex>
#include <QCheckBox>
#include <QLabel>
#include <QGridLayout>
#include <QStandardItem>

// Converts index all the way down to source index, going through whatever chain of proxies is present
QModelIndex convert_to_source(const QModelIndex &index) {
    if (!index.isValid()) {
        return index;
    }

    const QAbstractItemModel *current_model = index.model();
    QModelIndex current_index = index;

    while (true) {
        const QSortFilterProxyModel *proxy = qobject_cast<const QSortFilterProxyModel *>(current_model);

        if (proxy != nullptr) {
            current_index = proxy->mapToSource(current_index);

            current_model = proxy->sourceModel();
        } else {
            return current_index;
        }
    }
}

// Row index can be an index of any column in target row and of any proxy in the proxy chain
QString get_dn_from_index(const QModelIndex &base_row_index, int dn_column) {
    const QModelIndex row_index = convert_to_source(base_row_index);
    const QModelIndex dn_index = row_index.siblingAtColumn(dn_column);
    const QString dn = dn_index.data().toString();

    return dn;
}

// Set root index of view to head index of current model
// Do this to hide a head node in view while retaining it in model
// for drag and drop purposes
void set_root_to_head(QAbstractItemView *view) {
    const QAbstractItemModel *view_model = view->model();
    const QModelIndex head_index = view_model->index(0, 0);
    view->setRootIndex(head_index);
}

// Setup proxy chain down to source model
// And set view model to top proxy
void setup_model_chain(QAbstractItemView *view, QAbstractItemModel *source_model, QList<QAbstractProxyModel *> proxies) {
    for (int i = 0; i < proxies.size(); i++) {
        QAbstractItemModel *source;
        if (i == 0) {
            source = source_model;
        } else {
            source = proxies[i - 1];
        }

        QAbstractProxyModel *proxy = proxies[i];
        proxy->setSourceModel(source);
    }

    view->setModel(proxies.last());
}

bool checkbox_is_checked(const QCheckBox *checkbox) {
    return (checkbox->checkState() == Qt::Checked);
}

void checkbox_set_checked(QCheckBox *checkbox, bool checked) {
    Qt::CheckState check_state;
    if (checked) {
        check_state = Qt::Checked;
    } else {
        check_state = Qt::Unchecked;
    }
    
    checkbox->setCheckState(check_state);
}

void append_to_grid_layout_with_label(QGridLayout *layout,QLabel *label, QWidget *widget) {
    const int row = layout->rowCount();
    layout->addWidget(label, row, 0);
    layout->addWidget(widget, row, 1);
}

// If changed, return text with asterisk at the end
// If not changed, return text without asterisk
QString set_changed_marker(const QString &text, bool changed) {
    const int asterisk_index = text.indexOf("*");
    const bool has_asterisk = (asterisk_index != -1);

    QString new_text = text;
    if (changed && !has_asterisk) {
        new_text = text + "*";
    } else if (!changed && has_asterisk) {
        new_text = text;
        new_text.remove(asterisk_index, 1);
    }

    return new_text;
}

QList<QStandardItem *> make_item_row(const int count) {
    QList<QStandardItem *> row;

    for (int i = 0; i < count; i++) {
        const auto item = new QStandardItem();
        row.push_back(item);
    }

    return row;
}

int bit_set(int bitmask, int bit, bool set) {
    if (set) {
        return bitmask | bit;
    } else {
        return bitmask & ~bit;
    }
}

bool bit_is_set(int bitmask, int bit) {
    return ((bitmask & bit) != 0);
}
