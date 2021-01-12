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

#include "find_widget.h"

#include "ad_interface.h"
#include "ad_utils.h"
#include "settings.h"
#include "utils.h"
#include "filter.h"
#include "filter_widget/filter_widget.h"
#include "find_results.h"
#include "select_container_dialog.h"

#include <QString>
#include <QList>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QDebug>
#include <QCheckBox>

FindWidget::FindWidget(const QList<QString> classes, const QString &default_search_base)
: QWidget()
{
    // TODO: missing "Entire directory" in search base combo. Not 100% sure what it's supposed to be, the tippy-top domain? Definitely need it for work with multiple domains.

    search_base_combo = new QComboBox();
    {
        auto add_search_base_to_combo =
        [this](const QString dn) {
            const QString name = dn_get_name(dn);
            search_base_combo->addItem(name, dn);
        };

        if (default_search_base == AD()->domain_head()) {
            add_search_base_to_combo(AD()->domain_head());
        } else {
            add_search_base_to_combo(AD()->domain_head());
            add_search_base_to_combo(default_search_base);
        }

        const int last_index = search_base_combo->count() - 1;
        search_base_combo->setCurrentIndex(last_index);
    }

    auto custom_search_base_button = new QPushButton(tr("Browse"));
    custom_search_base_button->setAutoDefault(false);

    filter_widget = new FilterWidget(classes);

    auto quick_find_check = new QCheckBox(tr("Quick find"));

    auto find_button = new QPushButton(tr("Find"));
    find_button->setAutoDefault(false);

    auto stop_button = new QPushButton(tr("Stop"));
    stop_button->setAutoDefault(false);

    find_results = new FindResults();

    auto filter_widget_frame = new QFrame();
    filter_widget_frame->setFrameStyle(QFrame::Raised);
    filter_widget_frame->setFrameShape(QFrame::Box);

    {
        auto search_base_layout = new QHBoxLayout();
        search_base_layout->addWidget(search_base_combo);
        search_base_layout->addWidget(custom_search_base_button);

        auto search_base_row = new QFormLayout();
        search_base_row->addRow(tr("Search in:"), search_base_layout);

        auto buttons_layout = new QHBoxLayout();
        buttons_layout->addWidget(find_button);
        buttons_layout->addWidget(stop_button);
        buttons_layout->addStretch(1);

        auto layout = new QVBoxLayout();
        filter_widget_frame->setLayout(layout);
        layout->addLayout(search_base_row);
        layout->addWidget(filter_widget);
        layout->addWidget(quick_find_check);
        layout->addLayout(buttons_layout);
    }

    {
        auto layout = new QHBoxLayout();
        setLayout(layout);
        layout->addWidget(filter_widget_frame);
        layout->addWidget(find_results);
    }

    // Keep filter widget compact, so that when user
    // expands find dialog horizontally, filter widget will
    // keep it's size, find results will get expanded
    filter_widget_frame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);
    find_results->setMinimumSize(500, 0);

    connect(
        custom_search_base_button, &QAbstractButton::clicked,
        this, &FindWidget::select_custom_search_base);
    connect(
        find_button, &QPushButton::clicked,
        this, &FindWidget::find);
    connect(
        stop_button, &QPushButton::clicked,
        AD(), &AdInterface::stop_search);
    connect(
        filter_widget, &FilterWidget::return_pressed,
        this, &FindWidget::find);
    connect(
        filter_widget, &FilterWidget::changed,
        this, &FindWidget::on_filter_changed);

    SETTINGS()->connect_checkbox_to_bool_setting(quick_find_check, BoolSetting_QuickFind);
}

void FindWidget::select_custom_search_base() {
    auto dialog = new SelectContainerDialog(parentWidget());
    dialog->setWindowTitle(tr("Select custom search base"));

    connect(
        dialog, &SelectContainerDialog::accepted,
        [this, dialog]() {
            const QString selected = dialog->get_selected();
            const QString name = dn_get_name(selected);

            search_base_combo->addItem(name, selected);

            // Select newly added search base in combobox
            const int new_base_index = search_base_combo->count() - 1;
            search_base_combo->setCurrentIndex(new_base_index);
        });

    dialog->open();
}

void FindWidget::on_filter_changed() {
    const bool quick_find = SETTINGS()->get_bool(BoolSetting_QuickFind);

    if (quick_find) {
        find();
    }
}

void FindWidget::find() {
    const QString filter = filter_widget->get_filter();
    const QString search_base =
    [this]() {
        const int index = search_base_combo->currentIndex();
        const QVariant item_data = search_base_combo->itemData(index);

        return item_data.toString();
    }();

    find_results->load(filter, search_base);
}

QList<QList<QStandardItem *>> FindWidget::get_selected_rows() const {
    return find_results->get_selected_rows();
}
