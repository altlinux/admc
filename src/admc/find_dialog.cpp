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
#include "object_list_widget.h"
#include "select_dialog.h"

#include <QString>
#include <QList>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QTreeView>
#include <QSortFilterProxyModel>
#include <QComboBox>
#include <QPushButton>
#include <QItemSelectionModel>
#include <QStandardItemModel>
#include <QDebug>

FindDialog::FindDialog()
: QDialog()
{
    setWindowTitle(tr("Find dialog"));
    setAttribute(Qt::WA_DeleteOnClose);

    // TODO: technically, entire directory does NOT equal to the domain. In cases where we're browsing multiple domains at the same time (or maybe some other situations as well), we'd need "Entire directory" AND all of domains. Currently search base is set to domain anyway, so would need to start from reworking that.

    auto search_base_combo_label = new QLabel(tr("In:"));
    search_base_combo = new QComboBox();
    search_base_combo->addItem(tr("Entire directory"), AD()->search_base());

    const QString users_dn = "CN=Users," + AD()->search_base();
    search_base_combo->addItem("Users", users_dn);

    auto custom_search_base_button = new QPushButton(tr("Browse"));

    filter_widget = new FilterWidget();

    auto find_button = new QPushButton(tr("Find"));

    find_results = new ObjectListWidget();

    // Keep filter widget compact and expand find results.
    // As a result, when user expands find dialog
    // vertically, filter widget will keep it's size, find
    // results will get expanded
    filter_widget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    find_results->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);

    auto filter_widget_frame = new QFrame();
    filter_widget_frame->setFrameStyle(QFrame::Raised);
    filter_widget_frame->setFrameShape(QFrame::Box);
    
    {
        auto layout = new QGridLayout();
        filter_widget_frame->setLayout(layout);

        const int search_base_combo_row = layout->rowCount();
        layout->addWidget(search_base_combo_label, search_base_combo_row, 0);
        layout->addWidget(search_base_combo, search_base_combo_row, 1);
        layout->addWidget(custom_search_base_button, search_base_combo_row, 2);

        layout->addWidget(filter_widget, layout->rowCount(), 0, 1, layout->columnCount());
        layout->addWidget(find_button, layout->rowCount(), 1);
    }

    {
        auto layout = new QVBoxLayout();
        setLayout(layout);
        layout->addWidget(filter_widget_frame);
        layout->addWidget(find_results);
    }

    connect(
        custom_search_base_button, &QAbstractButton::clicked,
        this, &FindDialog::select_custom_search_base);
    connect(
        find_button, &QPushButton::clicked,
        this, &FindDialog::find);
}

void FindDialog::select_custom_search_base() {
    // TODO: maybe need some other classes?
    const QList<QString> selecteds = SelectDialog::open({CLASS_CONTAINER, CLASS_OU});

    if (!selecteds.isEmpty()) {
        const QString selected = selecteds[0];
        const QString name = dn_get_rdn(selected);

        search_base_combo->addItem(name, selected);

        // Select newly added search base in combobox
        const int new_base_index = search_base_combo->count() - 1;
        search_base_combo->setCurrentIndex(new_base_index);
    }
}

void FindDialog::find() {
    const QString filter = filter_widget->get_filter();
    const QString search_base =
    [this]() {
        const int index = search_base_combo->currentIndex();
        const QVariant item_data = search_base_combo->itemData(index);

        return item_data.toString();
    }();

    find_results->load_filter(filter, search_base);
}
