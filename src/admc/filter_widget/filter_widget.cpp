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

#include "filter_widget/filter_widget.h"
#include "ad_interface.h"
#include "ad_config.h"
#include "utils.h"
#include "select_dialog.h"
#include "filter.h"

#include <QDebug>
#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QCheckBox>
#include <QDialogButtonBox>

#include <algorithm>

enum Condition {
    Condition_Equals,
    Condition_NotEquals,
    Condition_StartsWith,
    Condition_EndsWith,
    Condition_Contains,
    Condition_COUNT,
};

const QList<QString> search_classes = {
    CLASS_USER,
    CLASS_GROUP,
    CLASS_CONTACT,
    CLASS_COMPUTER,
    CLASS_PRINTER,
    CLASS_OU,
    CLASS_TRUSTED_DOMAIN,
    CLASS_DOMAIN,
    CLASS_CONTAINER,
    CLASS_INET_ORG_PERSON,
    CLASS_FOREIGN_SECURITY_PRINCIPAL,
    CLASS_SHARED_FOLDER,
    CLASS_RPC_SERVICES,
    CLASS_CERTIFICATE_TEMPLATE,
    CLASS_MSMQ_GROUP,
    CLASS_MSMQ_QUEUE_ALIAS,
    CLASS_REMOTE_STORAGE_SERVICE,
};

QString condition_to_display_string(const Condition condition);

FilterWidget::FilterWidget()
: QWidget()
{
    advanced_tab = new QWidget();
    {
        ldap_filter_edit = new QPlainTextEdit(this);

        auto layout = new QVBoxLayout();
        advanced_tab->setLayout(layout);
        layout->addWidget(ldap_filter_edit);
    }

    normal_tab = new QWidget();
    {
        selected_classes_display = new QLineEdit();
        selected_classes_display->setReadOnly(true);

        auto select_classes_button = new QPushButton(tr("Select classes"));
        connect(
            select_classes_button, &QAbstractButton::clicked,
            this, &FilterWidget::on_select_classes);

        auto attribute_combo_label = new QLabel(tr("Attribute:"));

        attribute_class_combo = new QComboBox();
        for (const QString object_class : search_classes) {
            const QString display = ADCONFIG()->get_class_display_name(object_class);
            attribute_class_combo->addItem(display, object_class);
        }

        attribute_combo = new QComboBox();

        auto search_base_combo_label = new QLabel(tr("In:"));
        search_base_combo = new QComboBox();

        // TODO: technically, entire directory does NOT equal to the domain. In cases where we're browsing multiple domains at the same time (or maybe some other situations as well), we'd need "Entire directory" AND all of domains. Currently search base is set to domain anyway, so would need to start from reworking that.
        search_base_combo->addItem(tr("Entire directory"), AD()->search_base());

        const QString users_dn = "CN=Users," + AD()->search_base();
        search_base_combo->addItem("Users", users_dn);

        auto custom_search_base_button = new QPushButton(tr("Browse"));
        connect(
            custom_search_base_button, &QAbstractButton::clicked,
            this, &FilterWidget::on_custom_search_base);

        condition_combo = new QComboBox();
        for (int i = 0; i < Condition_COUNT; i++) {
            const Condition condition = (Condition) i;
            const QString condition_string = condition_to_display_string(condition);

            condition_combo->addItem(condition_string, i);
        }

        value_edit = new QLineEdit();

        auto add_filter_button = new QPushButton(tr("Add"));
        connect(
            add_filter_button, &QAbstractButton::clicked,
            this, &FilterWidget::on_add_filter);

        // Enable/disable add filter button depending on if value is filled
        connect(
            value_edit, &QLineEdit::textChanged,
            [this, add_filter_button]() {
                const bool value_filled = !value_edit->text().isEmpty();
                add_filter_button->setEnabled(value_filled);
            });
        add_filter_button->setEnabled(false);

        filter_list = new QListWidget();

        auto remove_filter_button = new QPushButton(tr("Remove"));
        connect(
            remove_filter_button, &QAbstractButton::clicked,
            this, &FilterWidget::on_remove_filter);

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
            this, &FilterWidget::on_clear_filters);

        auto filter_builder_wrapper = new QFrame();
        filter_builder_wrapper->setFrameStyle(QFrame::Raised);
        filter_builder_wrapper->setFrameShape(QFrame::Box);

        auto filter_builder_layout = new QGridLayout();
        filter_builder_wrapper->setLayout(filter_builder_layout);

        const int attribute_row = filter_builder_layout->rowCount();
        filter_builder_layout->addWidget(attribute_combo_label, attribute_row, 0);
        filter_builder_layout->addWidget(attribute_class_combo, attribute_row, 1);
        filter_builder_layout->addWidget(attribute_combo, attribute_row, 2);

        const int condition_value_row = filter_builder_layout->rowCount();
        filter_builder_layout->addWidget(condition_combo, condition_value_row, 0);
        filter_builder_layout->addWidget(value_edit, condition_value_row, 1, 1, 2);

        filter_builder_layout->addWidget(add_filter_button, filter_builder_layout->rowCount(), 0, 1, 3, Qt::AlignHCenter);

        auto layout = new QGridLayout();
        normal_tab->setLayout(layout);

        const int selected_classes_row = layout->rowCount();
        layout->addWidget(selected_classes_display, selected_classes_row, 0, 1, 2);
        layout->addWidget(select_classes_button, selected_classes_row, 2);

        const int search_base_row = layout->rowCount();
        layout->addWidget(search_base_combo_label, search_base_row, 0);
        layout->addWidget(search_base_combo, search_base_row, 1);
        layout->addWidget(custom_search_base_button, search_base_row, 2);

        layout->addWidget(filter_builder_wrapper, layout->rowCount(), 0, 1, 3);

        layout->addWidget(new QLabel(tr("Filters:")), layout->rowCount(), 0, 1, 3);
        layout->addWidget(filter_list, layout->rowCount(), 0, 1, 3);

        const int filter_list_buttons_row = layout->rowCount();
        layout->addWidget(remove_filter_button, filter_list_buttons_row, 0);
        layout->addWidget(clear_filters_button, filter_list_buttons_row, 1);
    }

    tab_widget = new QTabWidget();
    tab_widget->addTab(normal_tab, tr("Filter"));
    tab_widget->addTab(advanced_tab, tr("Advanced"));

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(tab_widget);

    connect(
        attribute_class_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterWidget::on_attribute_class_combo);
    on_attribute_class_combo();

    connect(
        ldap_filter_edit, &QPlainTextEdit::textChanged,
        [this]() {
            emit changed();
        });
}

// Fill attributes combo with attributes for selected class. Attributes are sorted by their display names.
void FilterWidget::on_attribute_class_combo() {
    attribute_combo->clear();

    const QString object_class =
    [this]() {
        const int index = attribute_class_combo->currentIndex();
        const QVariant item_data = attribute_class_combo->itemData(index);

        return item_data.toString();
    }();
    
    const QList<QString> attributes = 
    [this, object_class]() {
        const QList<QString> attributes_for_class = ADCONFIG()->get_find_attributes(object_class);
        const QList<QString> default_attributes = ADCONFIG()->get_find_attributes(CLASS_DEFAULT);

        QList<QString> out = attributes_for_class + default_attributes;
        out.removeDuplicates();
        
        return out;
    }();

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

void FilterWidget::on_custom_search_base() {
    // TODO: maybe need some other classes?
    const QList<QString> selecteds = SelectDialog::open({CLASS_CONTAINER, CLASS_OU});

    if (!selecteds.isEmpty()) {
        const QString selected = selecteds[0];
        const QString name = dn_get_rdn(selected);

        search_base_combo->addItem(name, selected);

        // Select newly added search base
        const int new_base_index = search_base_combo->count() - 1;
        search_base_combo->setCurrentIndex(new_base_index);
    }
}

void FilterWidget::on_add_filter() {
    const QString attribute = attribute_combo->itemData(attribute_combo->currentIndex()).toString();
    const Condition condition = (Condition) condition_combo->itemData(condition_combo->currentIndex()).toInt();
    const QString value = value_edit->text();

    const QString condition_string = condition_to_display_string(condition);
    const QString filter_display_string = QString("%1: \"%2\"").arg(condition_string, value);

    const QString filter =
    [attribute, condition, value]() {
        switch(condition) {
            case Condition_Equals: return filter_EQUALS(attribute, value);
            case Condition_NotEquals: return filter_NOT(filter_EQUALS(attribute, value));
            case Condition_StartsWith: return filter_EQUALS(attribute, "*" + value);
            case Condition_EndsWith: return filter_EQUALS(attribute, value + "*");
            case Condition_Contains: return filter_EQUALS(attribute, "*" + value + "*");
            case Condition_COUNT: return QString();
        }
        return QString();
    }();

    auto item = new QListWidgetItem();
    item->setText(filter_display_string);
    item->setData(Qt::UserRole, filter);
    filter_list->addItem(item);

    value_edit->clear();
}

void FilterWidget::on_select_classes() {
    auto dialog = new QDialog();
    dialog->setModal(true);

    auto layout = new QGridLayout();
    dialog->setLayout(layout);

    QHash<QString, QCheckBox *> checkboxes;

    for (const QString object_class : search_classes) {
        const QString class_display = ADCONFIG()->get_class_display_name(object_class);

        auto label = new QLabel(class_display);
        auto checkbox = new QCheckBox();

        const bool class_is_selected = selected_search_classes.contains(object_class);
        checkbox_set_checked(checkbox, class_is_selected);

        append_to_grid_layout_with_label(layout, label, checkbox);

        checkboxes[object_class] = checkbox;
    }

    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(
        button_box, &QDialogButtonBox::accepted,
        dialog, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        dialog, &QDialog::reject);
    layout->addWidget(button_box);

    connect(dialog, &QDialog::accepted,
        [this, checkboxes]() {
            selected_search_classes.clear();

            // Save selected classes
            for (const QString object_class : search_classes) {
                QCheckBox *checkbox = checkboxes[object_class];

                if (checkbox_is_checked(checkbox)) {
                    selected_search_classes.insert(object_class);
                }
            }

            // Display selected classes set as a sorted list
            // of class display strings separated by ","
            // {"user", "organizationUnit"}
            // =>
            // "User, Organizational Unit"
            const QString selected_classes_string =
            [this]() {
                QList<QString> selected_classes_display_strings;
                for (const QString object_class : selected_search_classes) {
                    const QString class_display = ADCONFIG()->get_class_display_name(object_class);
                    selected_classes_display_strings.append(class_display);
                }

                std::sort(selected_classes_display_strings.begin(), selected_classes_display_strings.end());

                const QString joined = selected_classes_display_strings.join(", ");

                return joined;
            }();
            selected_classes_display->setText(selected_classes_string);
        });

    dialog->open();
}

void FilterWidget::on_remove_filter() {
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

void FilterWidget::on_clear_filters() {
    filter_list->clear();
}

QString FilterWidget::get_filter() const {
    const QWidget *current_tab = tab_widget->currentWidget();

    if (current_tab == advanced_tab) {
        return ldap_filter_edit->toPlainText();
    } else {
        return QString();
    }

    return QString();
}

QString FilterWidget::get_search_base() const {
    const int index = search_base_combo->currentIndex();
    const QVariant item_data = search_base_combo->itemData(index);

    return item_data.toString();
}

QString condition_to_display_string(const Condition condition) {
    switch (condition) {
        case Condition_Equals: return QObject::tr("Equals");
        case Condition_NotEquals: return QObject::tr("Doesn't equal");
        case Condition_StartsWith: return QObject::tr("Starts with");
        case Condition_EndsWith: return QObject::tr("Ends with");
        case Condition_Contains: return QObject::tr("Contains");
        case Condition_COUNT: return QString();
    }
    return QString();
}
