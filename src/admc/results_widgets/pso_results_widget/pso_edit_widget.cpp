/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "pso_edit_widget.h"
#include "ad_defines.h"
#include "ui_pso_edit_widget.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_utils.h"
#include "select_dialogs/select_object_dialog.h"
#include "managers/icon_manager.h"
#include "status.h"
#include "globals.h"
#include "ad_config.h"
#include "utils.h"

#include <chrono>

PSOEditWidget::PSOEditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSOEditWidget) {
/**
* @brief Creates PSOEditWidget and initializes it with global values  
*/

    ui->setupUi(this);

    connect(ui->applied_list_widget, &QListWidget::itemSelectionChanged, this,
            [this]() {
            ui->remove_button->setDisabled(ui->applied_list_widget->count() == 0);
    });

    connect(ui->add_button, &QPushButton::clicked, this, &PSOEditWidget::on_add);
    connect(ui->remove_button, &QPushButton::clicked, this, &PSOEditWidget::on_remove);

    update_fields(global_password_settings());
}

PSOEditWidget::~PSOEditWidget() {
    delete ui;
}

/** 
* @brief Sets fields of PSOEditWidget according to a given object
* @param passwd_settings_obj The object with new values
* @details Object can be a PSO or object representing global password settings 
* those types of objects are distiguished based on objects' CN, if CN is present 
* the object is considered to be a PSO, othervise it's treated as global 
* settings object. If settings are global the following fields are not 
* displayed: name, precedence and list of users policy is applied to
*/
void PSOEditWidget::update(const AdObject &passwd_settings_obj) {
    if (is_global != !passwd_settings_obj.contains(ATTRIBUTE_CN)) {
        is_global = !passwd_settings_obj.contains(ATTRIBUTE_CN);
        ui->name_edit->setVisible(!is_global);
        ui->name_label->setVisible(!is_global);
        ui->precedence_label->setVisible(!is_global);
        ui->precedence_spinbox->setVisible(!is_global);
        ui->groupBox->setVisible(!is_global);
        ui->line->setVisible(!is_global);
        ui->groupBox_2->setTitle(is_global ? tr("Global password settings") :
                                             tr("Password settings"));
    }

    update_fields(passwd_settings_obj);
}

/**
* @brief Returns current values of widget fieds set by the user
* @return Hashmap of new values
*/
QHash<QString, QList<QByteArray>> PSOEditWidget::pso_settings_values() {
    using namespace std::chrono;

    QHash<QString, QList<QByteArray>> settings;

    settings[replace_attribute(ATTRIBUTE_CN)] = {ui->name_edit->text().trimmed().toUtf8()};

    settings[replace_attribute(ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE)] = {QByteArray::number(ui->precedence_spinbox->value())};
    settings[replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_LENGTH)] = {QByteArray::number(ui->min_passwd_len_spinbox->value())};
    settings[replace_attribute(ATTRIBUTE_MS_DS_PASSWORD_HISTORY_LENGTH)] = {QByteArray::number(ui->history_length_spinbox->value())};
    settings[replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_THRESHOLD)] = {QByteArray::number(ui->logon_attempts_spinbox->value())};

    settings[replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_DURATION)] = {
        QByteArray::number(-duration_cast<milliseconds>(
                               minutes(ui->lockout_duration_spinbox->value()))
                               .count() *
                           MILLIS_TO_100_NANOS)};
    settings[replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW)] = {
        QByteArray::number(-duration_cast<milliseconds>(
        minutes(ui->reset_lockout_spinbox->value())).count() * MILLIS_TO_100_NANOS)
    };

    settings[replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE)] = {
        QByteArray::number(-duration_cast<milliseconds>(
                               hours(24 * ui->min_age_spinbox->value()))
                               .count() *
                           MILLIS_TO_100_NANOS)};
    settings[replace_attribute(ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE)] = {
        QByteArray::number(-duration_cast<milliseconds>(
                               hours(24 * ui->max_age_spinbox->value()))
                               .count() *
                           MILLIS_TO_100_NANOS)};

    if (is_global) {
        settings[ATTRIBUTE_PWD_PROPERTIES] = {QByteArray::number(ui->complexity_req_checkbox->isChecked() * SAM_MASK_DOMAIN_PASSWORD_COMPLEX +
                                                                 ui->store_passwd_checkbox->isChecked() * SAM_MASK_DOMAIN_PASSWORD_STORE_CLEARTEXT)};
    } else {
        settings[ATTRIBUTE_MS_DS_PASSWORD_COMPLEXITY_ENABLED] = {
            QString(ui->complexity_req_checkbox->isChecked() ? LDAP_BOOL_TRUE :
                                                               LDAP_BOOL_FALSE)
                .toUtf8()};
        settings[ATTRIBUTE_MS_DS_PASSWORD_REVERSIBLE_ENCRYPTION_ENABLED] = {
            QString(ui->store_passwd_checkbox->isChecked() ? LDAP_BOOL_TRUE :
                                                             LDAP_BOOL_FALSE)
                .toUtf8()};
    }

    if (dn_applied_list.isEmpty()) {
        settings[ATTRIBUTE_PSO_APPLIES_TO] = QList<QByteArray>();
    } else {
        for (const QString &dn : dn_applied_list) {
            settings[ATTRIBUTE_PSO_APPLIES_TO].append(dn.toUtf8());
        }
    }

    return settings;
}

QHash<QString, QList<QString> > PSOEditWidget::pso_settings_string_values() {
/**
* @brief Returns current values of widget fieds set by the user
* @return Hashmap of new values
*/
    QHash<QString, QList<QString>> string_value_settings;
    QHash<QString, QList<QByteArray>> byte_value_settings = pso_settings_values();

    for (const QString &attr : byte_value_settings.keys()) {
        QList<QString> string_values;
        for (const QByteArray &value : byte_value_settings[attr]) {
            string_values.append(value);
        }
        string_value_settings[attr] = string_values;
    }

    return string_value_settings;
}

/**
* @brief Returns list of users to which the PSO applies
*/
QStringList PSOEditWidget::applied_dn_list() const {
    return dn_applied_list;
}

/**
* @brief Returns the name edit object
*/
QLineEdit *PSOEditWidget::name_line_edit() {
    return ui->name_edit;
}

/**
* @brief Compares current field values to their respective defaults (global 
* settings)
*/
bool PSOEditWidget::settings_are_default() {
    auto current_values = pso_settings_values();
    const QStringList excluded_attrs = {
        ATTRIBUTE_CN,
        ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE,
        ATTRIBUTE_APPLIES_TO};
    auto defaults = global_password_settings().get_attributes_data();
    for (const QString &attr : defaults.keys()) {
        if (excluded_attrs.contains(attr)) {
            continue;
        }

        if (defaults[pso_attributes_to_global_attributes[attr]] != current_values[attr]) {
            return false;
        }
    }

    return true;
}

void PSOEditWidget::on_add() {
    auto dialog = new SelectObjectDialog({CLASS_USER, CLASS_GROUP}, SelectObjectDialogMultiSelection_Yes, this);
    dialog->setWindowTitle(tr("Add applied users/group"));
    dialog->open();

    connect(dialog, &SelectObjectDialog::accepted, this, [this, dialog]() {
       for (auto selected_data : dialog->get_selected_advanced()) {
           QListWidgetItem *item = new QListWidgetItem(g_icon_manager->category_icon(dn_get_name(selected_data.category)),
                                                       dn_get_name(selected_data.dn),
                                                       ui->applied_list_widget);
           item->setData(AppliedItemRole_DN, selected_data.dn);
           dn_applied_list.append(selected_data.dn);
       }
    });
}

void PSOEditWidget::on_remove() {
    for (auto item : ui->applied_list_widget->selectedItems()) {
        dn_applied_list.removeAll(item->data(AppliedItemRole_DN).toString());
        delete item;
    }
}

void PSOEditWidget::set_read_only(bool read_only) {
    QList<QSpinBox*> spinbox_children = findChildren<QSpinBox*>(QString(), Qt::FindChildrenRecursively);
    ui->name_edit->setReadOnly(true);
    for (auto spinbox : spinbox_children) {
        spinbox->setReadOnly(read_only);
    }

    QList<QCheckBox*> checkbox_children = findChildren<QCheckBox*>(QString(), Qt::FindChildrenRecursively);
    for (auto checkbox : checkbox_children) {
        checkbox->setDisabled(read_only);
    }

    ui->add_button->setDisabled(read_only);
    // Only true because no items are selected after list widget enabling
    ui->remove_button->setDisabled(true);
    ui->applied_list_widget->setDisabled(read_only);
}

int PSOEditWidget::spinbox_timespan_units(const AdObject &obj, const QString &attribute) {
/**
* @brief Reads specified timespan attribute from given PSO object and converts 
* it to apropriate units
* @param obj PSO object to read attribute from
* @param attribute The attribute to retrieve
* @return Attribute value in specified units
*/
    using namespace std::chrono;

    qint64 hundred_nanos = -obj.get_value(attribute).toLongLong();
    milliseconds msecs(hundred_nanos / MILLIS_TO_100_NANOS);

    if (attribute ==
            replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW) ||
        attribute == replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_DURATION)) {
        int mins = duration_cast<minutes>(msecs).count();
        return mins;
    }

    if (attribute == replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE) ||
        attribute == replace_attribute(ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE)) {
        int days = duration_cast<hours>(msecs).count() / 24;
        return days;
    }

    return 0;
}

/**
* @brief Deduces apropriate attribute name from PSO atribute name based on 
* whether or not PSO is global. Basicaly method converts PSO attributes to 
* global password settings attributes if needed
* @param attribute_name Initial attribute to be converted
* @return The apropriate attribute considering object type
*/
QString PSOEditWidget::replace_attribute(QString attribute_name) {
    return is_global ? (pso_attributes_to_global_attributes.contains(attribute_name) ?
                                pso_attributes_to_global_attributes[attribute_name] :
                                QString()) :
                        attribute_name;
};

/** 
* @brief Sets fields of PSOEditWidget according to a given object
* @param passwd_settings_obj The object with new values
*/
void PSOEditWidget::update_fields(const AdObject &passwd_settings_obj) {
    ui->name_edit->setText(passwd_settings_obj.get_string(replace_attribute(ATTRIBUTE_CN)));

    ui->precedence_spinbox->setValue(passwd_settings_obj.get_int(replace_attribute(ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE)));
    ui->min_passwd_len_spinbox->setValue(passwd_settings_obj.get_int(replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_LENGTH)));
    ui->history_length_spinbox->setValue(passwd_settings_obj.get_int(replace_attribute(ATTRIBUTE_MS_DS_PASSWORD_HISTORY_LENGTH)));
    ui->logon_attempts_spinbox->setValue(passwd_settings_obj.get_int(replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_THRESHOLD)));

    ui->lockout_duration_spinbox->setValue(spinbox_timespan_units(passwd_settings_obj, replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_DURATION)));
    ui->reset_lockout_spinbox->setValue(spinbox_timespan_units(passwd_settings_obj, replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW)));
    ui->min_age_spinbox->setValue(spinbox_timespan_units(passwd_settings_obj, replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE)));
    ui->max_age_spinbox->setValue(spinbox_timespan_units(passwd_settings_obj, replace_attribute(ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE)));

    if (is_global) {
        int pwd_properties = passwd_settings_obj.get_int(ATTRIBUTE_PWD_PROPERTIES);
        ui->complexity_req_checkbox->setChecked(pwd_properties & SAM_MASK_DOMAIN_PASSWORD_COMPLEX);
        ui->store_passwd_checkbox->setChecked(pwd_properties & SAM_MASK_DOMAIN_PASSWORD_STORE_CLEARTEXT);
    } else {
        ui->complexity_req_checkbox->setChecked(passwd_settings_obj.get_bool(ATTRIBUTE_MS_DS_PASSWORD_COMPLEXITY_ENABLED));
        ui->store_passwd_checkbox->setChecked(passwd_settings_obj.get_bool(ATTRIBUTE_MS_DS_PASSWORD_REVERSIBLE_ENCRYPTION_ENABLED));
    }

    ui->applied_list_widget->clear();
    dn_applied_list = passwd_settings_obj.get_strings(ATTRIBUTE_PSO_APPLIES_TO);

    if (dn_applied_list.isEmpty()) {
        ui->remove_button->setDisabled(true);
        return;
    }

    AdInterface ad;
    if (!ad.is_connected()) {
        return;
    }

    for (const QString &dn : dn_applied_list) {
        AdObject applied_object = ad.search_object(dn, {ATTRIBUTE_OBJECT_CATEGORY});
        if (applied_object.is_empty()) {
            continue;
        }
        QListWidgetItem *item = new QListWidgetItem(g_icon_manager->object_icon(applied_object),
            dn_get_name(dn),
            ui->applied_list_widget);
        item->setData(AppliedItemRole_DN, dn);
    }
}
