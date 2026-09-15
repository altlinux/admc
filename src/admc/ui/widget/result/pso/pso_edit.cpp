/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2024-2025 Semyon Knyazev
 * Copyright (C) 2026 Artyom V. Poptsov
 * Copyright (C) 2026 Yuri Kozyrev
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

#include "ui/widget/result/pso/pso_edit.h"
#include "ui/widget/result/pso/ui_pso_edit.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_utils.h"
#include "ui/dialog/select/object.h"
#include "core/managers/icon_manager.h"
#include "ui/status.h"
#include "core/globals.h"
#include "ad_config.h"
#include "core/utils.h"

#include <chrono>

/**
* @brief Creates PSOEditWidget and initializes it with global values  
*/
PSOEditWidget::PSOEditWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PSOEditWidget) {

    ui->setupUi(this);

    connect(ui->applied_list_widget, &QListWidget::itemSelectionChanged, this,
            [this]() {
            ui->remove_button->setDisabled(ui->applied_list_widget->count() == 0);
    });

    connect(ui->add_button, &QPushButton::clicked, this, &PSOEditWidget::on_add);
    connect(ui->remove_button, &QPushButton::clicked, this, &PSOEditWidget::on_remove);

    update_fields(global_password_settings(), true);
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
    is_global = passwd_settings_obj.contains(ATTRIBUTE_CN);
    ui->name_edit->setVisible(!is_global);
    ui->name_label->setVisible(!is_global);
    ui->precedence_label->setVisible(!is_global);
    ui->precedence_spinbox->setVisible(!is_global);
    ui->groupBox->setVisible(!is_global);
    ui->line->setVisible(!is_global);
    ui->groupBox_2->setTitle(is_global ? tr("Global password settings") :
                                             tr("Password settings"));

    update_fields(passwd_settings_obj, is_global);
}

/**
* @brief Returns current values of widget fieds set by the user
* @return Hashmap of new values
*/
QHash<QString, QList<QByteArray>> PSOEditWidget::pso_settings_values() {
    using namespace std::chrono;

    QHash<QString, QList<QByteArray>> settings;

    settings[replace_attribute(ATTRIBUTE_CN, is_global)] = {
        ui->name_edit->text().trimmed().toUtf8()};

    settings[replace_attribute(ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE,
        is_global)] = {QByteArray::number(ui->precedence_spinbox->value())};
    settings[replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_LENGTH,
        is_global)] = {QByteArray::number(ui->min_passwd_len_spinbox->value())};
    settings[replace_attribute(ATTRIBUTE_MS_DS_PASSWORD_HISTORY_LENGTH,
        is_global)] = {QByteArray::number(ui->history_length_spinbox->value())};
    settings[replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_THRESHOLD, is_global)] =
        {QByteArray::number(ui->logon_attempts_spinbox->value())};

    settings[replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_DURATION, is_global)] = {
        minutes_to_ad_time_units(ui->lockout_duration_spinbox->value())};
    settings[replace_attribute(
        ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW, is_global)] = {
        minutes_to_ad_time_units(ui->reset_lockout_spinbox->value())};

    settings[replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE, is_global)] = {
        days_to_ad_time_units(ui->min_age_spinbox->value())};
    settings[replace_attribute(ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE, is_global)] = {
        days_to_ad_time_units(ui->max_age_spinbox->value())};

    if (is_global) {
        settings[ATTRIBUTE_PWD_PROPERTIES] = {
            QByteArray::number(ui->complexity_req_checkbox->isChecked() *
                                   SAM_MASK_DOMAIN_PASSWORD_COMPLEX +
                               ui->store_passwd_checkbox->isChecked() *
                                   SAM_MASK_DOMAIN_PASSWORD_STORE_CLEARTEXT)};
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
    const QSet<QString> excluded_attrs = {ATTRIBUTE_CN,
        ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE, ATTRIBUTE_PSO_APPLIES_TO,
        ATTRIBUTE_MS_DS_PASSWORD_COMPLEXITY_ENABLED,
        ATTRIBUTE_MS_DS_PASSWORD_REVERSIBLE_ENCRYPTION_ENABLED};
    auto defaults = global_password_settings().get_attributes_data();
    for (const QString &attr : current_values.keys()) {
        if (excluded_attrs.contains(attr)) {
            continue;
        }

        if (defaults[pso_attributes_to_global_attributes[attr]] !=
            current_values[attr]) {
            return false;
        }
    }
    int pwd_properties = defaults[ATTRIBUTE_PWD_PROPERTIES].toList()[0].toInt();
    if (bool(pwd_properties & SAM_MASK_DOMAIN_PASSWORD_COMPLEX) !=
        (current_values[ATTRIBUTE_MS_DS_PASSWORD_COMPLEXITY_ENABLED][0] ==
            LDAP_BOOL_TRUE)) {
        return false;
    }
    if (bool(pwd_properties & SAM_MASK_DOMAIN_PASSWORD_STORE_CLEARTEXT) !=
        (current_values[ATTRIBUTE_MS_DS_PASSWORD_REVERSIBLE_ENCRYPTION_ENABLED]
                       [0] == LDAP_BOOL_TRUE)) {
        return false;
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

/**
* @brief Converts time units used by AD to minutes
* @param value Value to be converted
* @return Converted value
*/
int PSOEditWidget::ad_time_units_to_mintes(const QByteArray &value) {
    using namespace std::chrono;
    return duration_cast<minutes>(
        milliseconds(ad_time_units_to_miliseconds(value)))
        .count();
}

/**
* @brief Converts time units used by AD to days
* @param value Value to be converted
* @return Converted value
*/
int PSOEditWidget::ad_time_units_to_days(const QByteArray &value) {
    using namespace std::chrono;
    return duration_cast<hours>(
               milliseconds(ad_time_units_to_miliseconds(value)))
               .count() /
           24;
}

/**
* @brief Converts time units used by AD to miliseconds
* @param value Value to be converted
* @return Converted value
*/
long long PSOEditWidget::ad_time_units_to_miliseconds(const QByteArray &value) {
    return -(value.toLongLong()) / MILLIS_TO_100_NANOS;
}

/**
* @brief Converts minutes to time units used by AD
* @param value Value to be converted
* @return Converted value
*/
QByteArray PSOEditWidget::minutes_to_ad_time_units(const int &value) {
    using namespace std::chrono;
    return miliseconds_to_ad_time_units(
        duration_cast<milliseconds>(minutes(value)).count());
}

/**
* @brief Converts days to time units used by AD
* @param value Value to be converted
* @return Converted value
*/
QByteArray PSOEditWidget::days_to_ad_time_units(const int &value) {
    using namespace std::chrono;
    return miliseconds_to_ad_time_units(
        duration_cast<milliseconds>(24 * hours(value)).count());
}

/**
* @brief Converts miliseconds to time units used by AD
* @param value Value to be converted
* @return Converted value
*/
QByteArray PSOEditWidget::miliseconds_to_ad_time_units(const long long &value) {
    return QByteArray::number(-value * MILLIS_TO_100_NANOS);
}

void PSOEditWidget::retranslate_ui() {
    qInfo() << "PSOEditWidget::retranslate_ui";
    ui->retranslateUi(this);
}

bool PSOEditWidget::event(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    return QObject::event(event);
}

/**
* @brief Deduces apropriate attribute name from PSO atribute name based on 
* whether or not PSO is global. Basicaly method converts PSO attributes to 
* global password settings attributes if needed
* @param attribute_name Initial attribute to be converted
* @return The apropriate attribute considering object type
*/
QString PSOEditWidget::replace_attribute(
    const QString &attribute_name, bool is_global) {
    if (!is_global) {
        return attribute_name;
    }
    if (pso_attributes_to_global_attributes.contains(attribute_name)) {
        return pso_attributes_to_global_attributes[attribute_name];
    }
    return QString();
};

/** 
* @brief Sets fields of PSOEditWidget according to a given object
* @param passwd_settings_obj The object with new values
*/
void PSOEditWidget::update_fields(
    const AdObject &passwd_settings_obj, const bool is_global) {
    ui->name_edit->setText(passwd_settings_obj.get_string(
        replace_attribute(ATTRIBUTE_CN, is_global)));

    ui->precedence_spinbox->setValue(
        passwd_settings_obj.get_int(replace_attribute(
            ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE, is_global)));
    ui->min_passwd_len_spinbox->setValue(passwd_settings_obj.get_int(
        replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_LENGTH, is_global)));
    ui->history_length_spinbox->setValue(passwd_settings_obj.get_int(
        replace_attribute(ATTRIBUTE_MS_DS_PASSWORD_HISTORY_LENGTH, is_global)));
    ui->logon_attempts_spinbox->setValue(passwd_settings_obj.get_int(
        replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_THRESHOLD, is_global)));

    ui->lockout_duration_spinbox->setValue(
        ad_time_units_to_mintes(passwd_settings_obj.get_value(
            replace_attribute(ATTRIBUTE_MS_DS_LOCKOUT_DURATION, is_global))));
    ui->reset_lockout_spinbox->setValue(
        ad_time_units_to_mintes(passwd_settings_obj.get_value(replace_attribute(
            ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW, is_global))));
    ui->min_age_spinbox->setValue(
        ad_time_units_to_days(passwd_settings_obj.get_value(
            replace_attribute(ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE, is_global))));
    ui->max_age_spinbox->setValue(
        ad_time_units_to_days(passwd_settings_obj.get_value(
            replace_attribute(ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE, is_global))));

    if (is_global) {
        int pwd_properties =
            passwd_settings_obj.get_int(ATTRIBUTE_PWD_PROPERTIES);
        ui->complexity_req_checkbox->setChecked(
            pwd_properties & SAM_MASK_DOMAIN_PASSWORD_COMPLEX);
        ui->store_passwd_checkbox->setChecked(
            pwd_properties & SAM_MASK_DOMAIN_PASSWORD_STORE_CLEARTEXT);
    } else {
        ui->complexity_req_checkbox->setChecked(passwd_settings_obj.get_bool(
            ATTRIBUTE_MS_DS_PASSWORD_COMPLEXITY_ENABLED));
        ui->store_passwd_checkbox->setChecked(passwd_settings_obj.get_bool(
            ATTRIBUTE_MS_DS_PASSWORD_REVERSIBLE_ENCRYPTION_ENABLED));
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
        AdObject applied_object =
            ad.search_object(dn, {ATTRIBUTE_OBJECT_CATEGORY});
        if (applied_object.is_empty()) {
            continue;
        }
        QListWidgetItem *item =
            new QListWidgetItem(g_icon_manager->object_icon(applied_object),
                dn_get_name(dn), ui->applied_list_widget);
        item->setData(AppliedItemRole_DN, dn);
    }
}
