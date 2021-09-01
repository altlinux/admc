/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#include "filter_widget/filter_builder.h"

#include "adldap.h"

#include <QComboBox>
#include <QCoreApplication>
#include <QFormLayout>
#include <QLineEdit>

#include <algorithm>

QString condition_to_display_string(const Condition condition);

FilterBuilder::FilterBuilder(AdConfig *adconfig_arg)
: QWidget() {
    adconfig = adconfig_arg;

    attribute_class_combo = new QComboBox();
    for (const QString &object_class : filter_classes) {
        const QString display = adconfig->get_class_display_name(object_class);
        attribute_class_combo->addItem(display, object_class);
    }

    attribute_combo = new QComboBox();

    condition_combo = new QComboBox();

    value_edit = new QLineEdit();

    auto layout = new QFormLayout();
    setLayout(layout);

    layout->addRow(tr("Attribute class:"), attribute_class_combo);
    layout->addRow(tr("Attribute:"), attribute_combo);
    layout->addRow(tr("Condition:"), condition_combo);
    layout->addRow(tr("Value:"), value_edit);

    connect(
        attribute_class_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterBuilder::update_attributes_combo);
    update_attributes_combo();

    connect(
        attribute_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterBuilder::update_conditions_combo);
    update_conditions_combo();

    connect(
        condition_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterBuilder::update_value_edit);
    update_value_edit();

    value_edit->setObjectName("value_edit");
}

QString FilterBuilder::get_filter() const {
    const QString attribute = attribute_combo->itemData(attribute_combo->currentIndex()).toString();
    const Condition condition = (Condition) condition_combo->itemData(condition_combo->currentIndex()).toInt();
    const QString value = value_edit->text();

    const QString filter_display_string = [this, condition, value]() {
        const QString attribute_display = attribute_combo->itemText(attribute_combo->currentIndex());
        const QString condition_string = condition_to_display_string(condition);

        const bool set_unset_condition = (condition == Condition_Set || condition == Condition_Unset);
        if (set_unset_condition) {
            return QString("%1 %2").arg(attribute_display, condition_string);
        } else {
            return QString("%1 %2: \"%3\"").arg(attribute_display, condition_string, value);
        }
    }();

    return filter_CONDITION(condition, attribute, value);
}

QString FilterBuilder::get_filter_display() const {
    const QString attribute = attribute_combo->itemData(attribute_combo->currentIndex()).toString();
    const Condition condition = (Condition) condition_combo->itemData(condition_combo->currentIndex()).toInt();
    const QString value = value_edit->text();

    const QString attribute_display = attribute_combo->itemText(attribute_combo->currentIndex());
    const QString condition_string = condition_to_display_string(condition);

    const bool set_unset_condition = (condition == Condition_Set || condition == Condition_Unset);
    if (set_unset_condition) {
        return QString("%1 %2").arg(attribute_display, condition_string);
    } else {
        return QString("%1 %2: \"%3\"").arg(attribute_display, condition_string, value);
    }
}

void FilterBuilder::clear() {
    value_edit->clear();
}

// Attributes combo contents depend on what attribute class is selected
void FilterBuilder::update_attributes_combo() {
    attribute_combo->clear();

    const QString object_class = [this]() {
        const int index = attribute_class_combo->currentIndex();
        const QVariant item_data = attribute_class_combo->itemData(index);

        return item_data.toString();
    }();

    const QList<QString> attributes = adconfig->get_find_attributes(object_class);

    const QList<QString> display_attributes = [&]() {
        QList<QString> out;

        for (const QString &attribute : attributes) {
            const QString display_name = adconfig->get_attribute_display_name(attribute, object_class);
            out.append(display_name);
        }

        std::sort(out.begin(), out.end());

        return out;
    }();

    // NOTE: need backwards mapping from display name to attribute for insertion
    const QHash<QString, QString> display_to_attribute = [&]() {
        QHash<QString, QString> out;
        for (const QString &attribute : attributes) {
            const QString display_name = adconfig->get_attribute_display_name(attribute, object_class);

            out[display_name] = attribute;
        }
        return out;
    }();

    // Insert attributes into combobox in the sorted order of display attributes
    for (const auto &display_attribute : display_attributes) {
        const QString attribute = display_to_attribute[display_attribute];
        attribute_combo->addItem(display_attribute, attribute);
    }
}

// Conditions combo contents depend on what attribute is selected
void FilterBuilder::update_conditions_combo() {
    const QList<Condition> conditions = [this]() -> QList<Condition> {
        const AttributeType attribute_type = [this]() {
            const int index = attribute_combo->currentIndex();
            const QVariant item_data = attribute_combo->itemData(index);
            const QString attribute = item_data.toString();

            return adconfig->get_attribute_type(attribute);
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

    condition_combo->clear();
    for (const auto condition : conditions) {
        const QString condition_string = condition_to_display_string(condition);

        condition_combo->addItem(condition_string, (int) condition);
    }
}

// Value edit is turned off for set/unset conditions
void FilterBuilder::update_value_edit() {
    const Condition condition = (Condition) condition_combo->itemData(condition_combo->currentIndex()).toInt();

    const bool disable_value_edit = (condition == Condition_Set || condition == Condition_Unset);
    value_edit->setDisabled(disable_value_edit);

    if (disable_value_edit) {
        value_edit->clear();
    }
}
