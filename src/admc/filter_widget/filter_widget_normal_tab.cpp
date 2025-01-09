/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
#include "filter_widget/ui_filter_widget_normal_tab.h"

#include "adldap.h"
#include "globals.h"

#include <algorithm>

FilterWidgetNormalTab::FilterWidgetNormalTab()
: FilterWidgetTab() {
    ui = new Ui::FilterWidgetNormalTab();
    ui->setupUi(this);

    connect(
        ui->attribute_class_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterWidgetNormalTab::update_attributes_combo);
    connect(
        ui->attribute_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterWidgetNormalTab::update_conditions_combo);
    connect(
        ui->condition_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterWidgetNormalTab::update_value_edit);
    connect(
        ui->remove_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::remove_filter);
    connect(
        ui->add_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::add_filter);
    connect(
        ui->clear_button, &QAbstractButton::clicked,
        this, &FilterWidgetNormalTab::clear_filters);
}

FilterWidgetNormalTab::~FilterWidgetNormalTab() {
    delete ui;
}

void FilterWidgetNormalTab::set_classes(const QList<QString> &class_list, const QList<QString> &selected_list) {
    for (const QString &object_class : filter_classes) {
        const QString display = g_adconfig->get_class_display_name(object_class);
        ui->attribute_class_combo->addItem(display, object_class);
    }

    ui->select_classes_widget->set_classes(class_list, selected_list);
}

void FilterWidgetNormalTab::enable_filtering_all_classes() {
    ui->select_classes_widget->enable_filtering_all_classes();
}

QString FilterWidgetNormalTab::get_filter() const {
    const QString attribute_filter = [this]() {
        QList<QString> filters;
        for (int i = 0; i < ui->filter_list->count(); i++) {
            const QListWidgetItem *item = ui->filter_list->item(i);
            const QString filter = item->data(Qt::UserRole).toString();

            filters.append(filter);
        }

        return filter_AND(filters);
    }();

    const QString class_filter = ui->select_classes_widget->get_filter();

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

void FilterWidgetNormalTab::clear() {
    ui->attribute_class_combo->setCurrentIndex(0);
    ui->value_edit->clear();

    clear_filters();
}

void FilterWidgetNormalTab::add_filter() {
    const QString filter = [&]() {
        const QString attribute = ui->attribute_combo->itemData(ui->attribute_combo->currentIndex()).toString();
        const Condition condition = (Condition) ui->condition_combo->itemData(ui->condition_combo->currentIndex()).toInt();
        const QString value = ui->value_edit->text();

        const QString filter_display_string = [this, condition, value]() {
            const QString attribute_display = ui->attribute_combo->itemText(ui->attribute_combo->currentIndex());
            const QString condition_string = condition_to_display_string(condition);

            const bool set_unset_condition = (condition == Condition_Set || condition == Condition_Unset);
            if (set_unset_condition) {
                return QString("%1 %2").arg(attribute_display, condition_string);
            } else {
                return QString("%1 %2: \"%3\"").arg(attribute_display, condition_string, value);
            }
        }();

        return filter_CONDITION(condition, attribute, value);
    }();
    const QString filter_display = [&]() {
        const QString attribute = ui->attribute_combo->itemData(ui->attribute_combo->currentIndex()).toString();
        const Condition condition = (Condition) ui->condition_combo->itemData(ui->condition_combo->currentIndex()).toInt();
        const QString value = ui->value_edit->text();

        const QString attribute_display = ui->attribute_combo->itemText(ui->attribute_combo->currentIndex());
        const QString condition_string = condition_to_display_string(condition);

        const bool set_unset_condition = (condition == Condition_Set || condition == Condition_Unset);
        if (set_unset_condition) {
            return QString("%1 %2").arg(attribute_display, condition_string);
        } else {
            return QString("%1 %2: \"%3\"").arg(attribute_display, condition_string, value);
        }
    }();

    auto item = new QListWidgetItem();
    item->setText(filter_display);
    item->setData(Qt::UserRole, filter);
    ui->filter_list->addItem(item);

    ui->value_edit->clear();
}

void FilterWidgetNormalTab::remove_filter() {
    const QList<QListWidgetItem *> selected_items = ui->filter_list->selectedItems();

    for (auto item : selected_items) {
        delete item;
    }
}

void FilterWidgetNormalTab::clear_filters() {
    ui->filter_list->clear();
}

QVariant FilterWidgetNormalTab::save_state() const {
    QHash<QString, QVariant> state;

    state["select_classes_widget"] = ui->select_classes_widget->save_state();

    QList<QString> filter_display_list;
    QList<QString> filter_value_list;
    for (int i = 0; i < ui->filter_list->count(); i++) {
        const QListWidgetItem *item = ui->filter_list->item(i);
        const QString filter_display = item->data(Qt::DisplayRole).toString();
        const QString filter_value = item->data(Qt::UserRole).toString();

        filter_display_list.append(filter_display);
        filter_value_list.append(filter_value);
    }

    state["filter_display_list"] = QVariant(filter_display_list);
    state["filter_value_list"] = QVariant(filter_value_list);

    return QVariant(state);
}

void FilterWidgetNormalTab::restore_state(const QVariant &state_variant) {
    const QHash<QString, QVariant> state = state_variant.toHash();

    ui->select_classes_widget->restore_state(state["select_classes_widget"]);

    const QList<QString> filter_display_list = state["filter_display_list"].toStringList();
    const QList<QString> filter_value_list = state["filter_value_list"].toStringList();

    ui->filter_list->clear();
    for (int i = 0; i < filter_display_list.size(); i++) {
        const QString filter_display = filter_display_list[i];
        const QString filter_value = filter_value_list[i];

        auto item = new QListWidgetItem();
        item->setText(filter_display);
        item->setData(Qt::UserRole, filter_value);
        ui->filter_list->addItem(item);
    }
}

// Attributes combo contents depend on what attribute class is selected
void FilterWidgetNormalTab::update_attributes_combo() {
    ui->attribute_combo->clear();

    const QString object_class = [this]() {
        const int index = ui->attribute_class_combo->currentIndex();
        const QVariant item_data = ui->attribute_class_combo->itemData(index);

        return item_data.toString();
    }();

    const QList<QString> attributes = g_adconfig->get_find_attributes(object_class);

    const QList<QString> display_attributes = [&]() {
        QList<QString> out;

        for (const QString &attribute : attributes) {
            const QString display_name = g_adconfig->get_attribute_display_name(attribute, object_class);
            out.append(display_name);
        }

        std::sort(out.begin(), out.end());

        return out;
    }();

    // NOTE: need backwards mapping from display name to attribute for insertion
    const QHash<QString, QString> display_to_attribute = [&]() {
        QHash<QString, QString> out;
        for (const QString &attribute : attributes) {
            const QString display_name = g_adconfig->get_attribute_display_name(attribute, object_class);

            out[display_name] = attribute;
        }
        return out;
    }();

    // Insert attributes into combobox in the sorted order of display attributes
    for (const auto &display_attribute : display_attributes) {
        const QString attribute = display_to_attribute[display_attribute];
        ui->attribute_combo->addItem(display_attribute, attribute);
    }
}

// Conditions combo contents depend on what attribute is selected
void FilterWidgetNormalTab::update_conditions_combo() {
    const QList<Condition> conditions = [this]() -> QList<Condition> {
        const AttributeType attribute_type = [this]() {
            const int index = ui->attribute_combo->currentIndex();
            const QVariant item_data = ui->attribute_combo->itemData(index);
            const QString attribute = item_data.toString();

            return g_adconfig->get_attribute_type(attribute);
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
                Condition_StartsWith,
                Condition_EndsWith,
                Condition_Equals,
                Condition_NotEquals,
                Condition_Set,
                Condition_Unset,
            };
        }
    }();

    ui->condition_combo->clear();
    for (const auto condition : conditions) {
        const QString condition_string = condition_to_display_string(condition);

        ui->condition_combo->addItem(condition_string, (int) condition);
    }
}

// Value edit is turned off for set/unset conditions
void FilterWidgetNormalTab::update_value_edit() {
    const Condition condition = (Condition) ui->condition_combo->itemData(ui->condition_combo->currentIndex()).toInt();

    const bool disable_value_edit = (condition == Condition_Set || condition == Condition_Unset);
    ui->value_edit->setDisabled(disable_value_edit);

    if (disable_value_edit) {
        ui->value_edit->clear();
    }
}
