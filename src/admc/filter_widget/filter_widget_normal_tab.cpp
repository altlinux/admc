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

#include "filter_widget/filter_widget_normal_tab.h"
#include "filter_widget/select_classes_widget.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "utils.h"
#include "filter.h"

#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QDebug>

#include <algorithm>

QString condition_to_display_string(const Condition condition);

FilterWidgetNormalTab::FilterWidgetNormalTab()
: FilterWidgetTab()
{
    select_classes = new SelectClassesWidget();

    attribute_class_combo = new QComboBox();
    for (const QString object_class : search_classes) {
        const QString display = ADCONFIG()->get_class_display_name(object_class);
        attribute_class_combo->addItem(display, object_class);
    }

    attribute_combo = new QComboBox();

    condition_combo = new QComboBox();

    value_edit = new QLineEdit();

    auto add_filter_button = new QPushButton(tr("Add"));
    connect(
        add_filter_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::on_add_filter);

    filter_list = new QListWidget();

    auto remove_filter_button = new QPushButton(tr("Remove"));
    connect(
        remove_filter_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::on_remove_filter);

    const QItemSelectionModel *filters_selection_model = filter_list->selectionModel();

    // Enable/disable remove filter button depending on
    // if any filter is currently selected
    connect(
        filters_selection_model, &QItemSelectionModel::selectionChanged,
        [remove_filter_button, filters_selection_model]() {
            const QList<QModelIndex> selecteds = filters_selection_model->selectedIndexes();
            const bool any_selected = !selecteds.isEmpty();

            remove_filter_button->setEnabled(any_selected);
        });
    remove_filter_button->setEnabled(false);

    auto clear_filters_button = new QPushButton(tr("Clear"));
    connect(
        clear_filters_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::on_clear_filters);

    auto filter_builder_wrapper = new QFrame();
    filter_builder_wrapper->setFrameStyle(QFrame::Raised);
    filter_builder_wrapper->setFrameShape(QFrame::Box);

    auto filter_builder_layout = new QGridLayout();
    filter_builder_wrapper->setLayout(filter_builder_layout);

    const int attribute_labels_row = filter_builder_layout->rowCount();
    filter_builder_layout->addWidget(new QLabel(tr("Attribute class:")), attribute_labels_row, 0);
    filter_builder_layout->addWidget(new QLabel(tr("Attribute:")), attribute_labels_row, 1);

    const int attribute_row = filter_builder_layout->rowCount();
    filter_builder_layout->addWidget(attribute_class_combo, attribute_row, 0);
    filter_builder_layout->addWidget(attribute_combo, attribute_row, 1, 1, 2);

    const int condition_value_labels_row = filter_builder_layout->rowCount();
    filter_builder_layout->addWidget(new QLabel(tr("Condition:")), condition_value_labels_row, 0);
    filter_builder_layout->addWidget(new QLabel(tr("Value:")), condition_value_labels_row, 1);

    const int condition_value_row = filter_builder_layout->rowCount();
    filter_builder_layout->addWidget(condition_combo, condition_value_row, 0);
    filter_builder_layout->addWidget(value_edit, condition_value_row, 1, 1, 2);

    filter_builder_layout->addWidget(add_filter_button, filter_builder_layout->rowCount(), 0, 1, 3, Qt::AlignHCenter);

    auto layout = new QGridLayout();
    setLayout(layout);

    const int select_classes_row = layout->rowCount();
    layout->addWidget(select_classes, select_classes_row, 0, 1, 3);

    layout->addWidget(filter_builder_wrapper, layout->rowCount(), 0, 1, 3);

    layout->addWidget(new QLabel(tr("Filters:")), layout->rowCount(), 0, 1, 3);
    layout->addWidget(filter_list, layout->rowCount(), 0, 1, 3);

    const int filter_list_buttons_row = layout->rowCount();
    layout->addWidget(remove_filter_button, filter_list_buttons_row, 0);
    layout->addWidget(clear_filters_button, filter_list_buttons_row, 1);

    connect(
        attribute_class_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterWidgetNormalTab::on_attribute_class_combo);
    on_attribute_class_combo();

    connect(
        attribute_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterWidgetNormalTab::fill_conditions_combo);
    fill_conditions_combo();

    connect(
        condition_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterWidgetNormalTab::on_condition_combo);

}

QString FilterWidgetNormalTab::get_filter() const {
    const QString attribute_filter =
    [this]() {
        QList<QString> filters;
        for (int i = 0; i < filter_list->count(); i++) {
            const QListWidgetItem *item = filter_list->item(i);
            const QString filter = item->data(Qt::UserRole).toString();

            filters.append(filter);
        }

        return filter_AND(filters);
    }();

    const QString class_filter = select_classes->get_filter();

    const bool classes = !class_filter.isEmpty();
    const bool attributes = !attribute_filter.isEmpty();

    if (classes && attributes) {
        return filter_AND({class_filter, attribute_filter});
    } else if (!classes && attributes) {
        return attribute_filter;
    } else if (classes && !attributes) {
        return class_filter;
    } else {
        return QString();
    }
}

// Fill attributes combo with attributes for selected class. Attributes are sorted by their display names.
void FilterWidgetNormalTab::on_attribute_class_combo() {
    attribute_combo->clear();

    const QString object_class =
    [this]() {
        const int index = attribute_class_combo->currentIndex();
        const QVariant item_data = attribute_class_combo->itemData(index);

        return item_data.toString();
    }();
    
    const QList<QString> attributes = ADCONFIG()->get_find_attributes(object_class);

    const QList<QString> display_attributes =
    [attributes, object_class]() {
        QList<QString> out;

        for (const QString attribute : attributes) {
            const QString display_name = ADCONFIG()->get_attribute_display_name(attribute, object_class);
            out.append(display_name);
        }

        std::sort(out.begin(), out.end());

        return out;
    }();

    // NOTE: need backwards mapping from display name to attribute for insertion
    const QHash<QString, QString> display_to_attribute =
    [attributes, object_class]() {
        QHash<QString, QString> out;
        for (const QString attribute : attributes) {
            const QString display_name = ADCONFIG()->get_attribute_display_name(attribute, object_class);

            out[display_name] = attribute;
        }
        return out;
    }();

    // Insert attributes into combobox in the sorted order of display attributes
    for (const auto display_attribute : display_attributes) {
        const QString attribute = display_to_attribute[display_attribute];
        attribute_combo->addItem(display_attribute, attribute);
    }
}

void FilterWidgetNormalTab::fill_conditions_combo() {
    const QList<Condition> conditions =
    [this]() -> QList<Condition> {
        const AttributeType attribute_type =
        [this]() {
            const int index = attribute_combo->currentIndex();
            const QVariant item_data = attribute_combo->itemData(index);
            const QString attribute = item_data.toString();

            return ADCONFIG()->get_attribute_type(attribute);
        }();

        // NOTE: extra conditions don't work on DSDN type
        // attributes, so don't include them in the combobox
        // in that case
        if (attribute_type == AttributeType_DSDN) {
            return {
                Condition_Equals,
                Condition_NotEquals,
                Condition_Set,
                Condition_Unset,
            };
        } else {
            return {
                Condition_Contains,
                Condition_StartsWith,
                Condition_EndsWith,
                Condition_Equals,
                Condition_NotEquals,
                Condition_Set,
                Condition_Unset,
            };
        }
    }();

    condition_combo->clear();
    for (const auto condition : conditions) {
        const QString condition_string = condition_to_display_string(condition);

        condition_combo->addItem(condition_string, (int)condition);
    }
}

void FilterWidgetNormalTab::on_add_filter() {
    const QString attribute = attribute_combo->itemData(attribute_combo->currentIndex()).toString();
    const Condition condition = (Condition) condition_combo->itemData(condition_combo->currentIndex()).toInt();
    const QString value = value_edit->text();

    const QString filter_display_string =
    [this, condition, value]() {
        const QString attribute_display = attribute_combo->itemText(attribute_combo->currentIndex());
        const QString condition_string = condition_to_display_string(condition);
        
        const bool set_unset_condition = (condition == Condition_Set || condition == Condition_Unset);
        if (set_unset_condition) {
            return QString("%1 %2").arg(attribute_display, condition_string);
        } else {
            return QString("%1 %2: \"%3\"").arg(attribute_display, condition_string, value);
        }
    }();

    const QString filter = filter_CONDITION(condition, attribute, value);

    auto item = new QListWidgetItem();
    item->setText(filter_display_string);
    item->setData(Qt::UserRole, filter);
    filter_list->addItem(item);

    value_edit->clear();
}

void FilterWidgetNormalTab::on_remove_filter() {
    const QSet<QListWidgetItem *> removed_items =
    [this]() {
        QSet<QListWidgetItem *> out;

        const QItemSelectionModel *selection_model = filter_list->selectionModel();
        const QList<QModelIndex> selecteds = selection_model->selectedIndexes();

        for (const auto selected : selecteds) {
            const int row = selected.row();
            QListWidgetItem *item = filter_list->item(row);

            out.insert(item);
        }

        return out;
    }();

    for (auto item : removed_items) {
        delete item;
    }
}

void FilterWidgetNormalTab::on_clear_filters() {
    filter_list->clear();
}

void FilterWidgetNormalTab::on_condition_combo() {
    const Condition condition = (Condition) condition_combo->itemData(condition_combo->currentIndex()).toInt();

    const bool disable_value_edit = (condition == Condition_Set || condition == Condition_Unset);
    value_edit->setDisabled(disable_value_edit);

    if (disable_value_edit) {
        value_edit->clear();
    }
}

QString condition_to_display_string(const Condition condition) {
    switch (condition) {
        case Condition_Equals: return QObject::tr("Equals");
        case Condition_NotEquals: return QObject::tr("Doesn't equal");
        case Condition_StartsWith: return QObject::tr("Starts with");
        case Condition_EndsWith: return QObject::tr("Ends with");
        case Condition_Contains: return QObject::tr("Contains");
        case Condition_Set: return QObject::tr("Set");
        case Condition_Unset: return QObject::tr("Unset");
        case Condition_COUNT: return QString();
    }
    return QString();
}
