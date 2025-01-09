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
#include "ui_pso_edit_widget.h"
#include "ad_interface.h"
#include "ad_object.h"
#include "ad_utils.h"
#include "select_dialogs/select_object_dialog.h"
#include "managers/icon_manager.h"
#include "status.h"
#include "globals.h"

#include <chrono>

#include <QDebug>

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

    update_defaults();
    default_setting_values = pso_settings_values();
}

PSOEditWidget::~PSOEditWidget() {
    delete ui;
}

void PSOEditWidget::update(const AdObject &passwd_settings_obj) {
    ui->name_edit->setText(passwd_settings_obj.get_string(ATTRIBUTE_CN));
    ui->name_edit->setReadOnly(true);

    ui->precedence_spinbox->setValue(passwd_settings_obj.get_int(ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE));
    ui->min_passwd_len_spinbox->setValue(passwd_settings_obj.get_int(ATTRIBUTE_MS_DS_MIN_PASSWORD_LENGTH));
    ui->history_length_spinbox->setValue(passwd_settings_obj.get_int(ATTRIBUTE_MS_DS_PASSWORD_HISTORY_LENGTH));
    ui->logon_attempts_spinbox->setValue(passwd_settings_obj.get_int(ATTRIBUTE_MS_DS_LOCKOUT_THRESHOLD));

    ui->lockout_duration_spinbox->setValue(spinbox_timespan_units(passwd_settings_obj, ATTRIBUTE_MS_DS_LOCKOUT_DURATION));
    ui->reset_lockout_spinbox->setValue(spinbox_timespan_units(passwd_settings_obj, ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW));
    ui->min_age_spinbox->setValue(spinbox_timespan_units(passwd_settings_obj, ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE));
    ui->max_age_spinbox->setValue(spinbox_timespan_units(passwd_settings_obj, ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE));

    ui->complexity_req_checkbox->setChecked(passwd_settings_obj.get_bool(ATTRIBUTE_MS_DS_PASSWORD_COMPLEXITY_ENABLED));
    ui->store_passwd_checkbox->setChecked(passwd_settings_obj.get_bool(ATTRIBUTE_MS_DS_PASSWORD_REVERSIBLE_ENCRYPTION_ENABLED));

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
        QListWidgetItem *item = new QListWidgetItem(g_icon_manager->get_object_icon(applied_object),
                                                    dn_get_name(dn),
                                                    ui->applied_list_widget);
        item->setData(AppliedItemRole_DN, dn);
    }
}

QHash<QString, QList<QByteArray>> PSOEditWidget::pso_settings_values() {
    using namespace std::chrono;

    QHash<QString, QList<QByteArray>> settings;

    settings[ATTRIBUTE_CN] = {ui->name_edit->text().trimmed().toUtf8()};

    settings[ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE] = {QByteArray::number(ui->precedence_spinbox->value())};
    settings[ATTRIBUTE_MS_DS_MIN_PASSWORD_LENGTH] = {QByteArray::number(ui->min_passwd_len_spinbox->value())};
    settings[ATTRIBUTE_MS_DS_PASSWORD_HISTORY_LENGTH] = {QByteArray::number(ui->history_length_spinbox->value())};
    settings[ATTRIBUTE_MS_DS_LOCKOUT_THRESHOLD] = {QByteArray::number(ui->logon_attempts_spinbox->value())};

    settings[ATTRIBUTE_MS_DS_LOCKOUT_DURATION] = {
        QByteArray::number(-duration_cast<milliseconds>(
        minutes(ui->lockout_duration_spinbox->value())).count() * MILLIS_TO_100_NANOS)
    };
    settings[ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW] = {
        QByteArray::number(-duration_cast<milliseconds>(
        minutes(ui->reset_lockout_spinbox->value())).count() * MILLIS_TO_100_NANOS)
    };

    settings[ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE] = {
        QByteArray::number(-duration_cast<milliseconds>(
        hours(24 * ui->min_age_spinbox->value())).count() * MILLIS_TO_100_NANOS)
    };
    settings[ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE] = {
        QByteArray::number(-duration_cast<milliseconds>(
        hours(24 * ui->max_age_spinbox->value())).count() * MILLIS_TO_100_NANOS)
    };

    settings[ATTRIBUTE_MS_DS_PASSWORD_COMPLEXITY_ENABLED] = {
        QString(ui->complexity_req_checkbox->isChecked() ? LDAP_BOOL_TRUE :
        LDAP_BOOL_FALSE).toUtf8()
    };
    settings[ATTRIBUTE_MS_DS_PASSWORD_REVERSIBLE_ENCRYPTION_ENABLED] = {
        QString(ui->store_passwd_checkbox->isChecked() ? LDAP_BOOL_TRUE :
        LDAP_BOOL_FALSE).toUtf8()
    };

    for (const QString &dn : dn_applied_list) {
        settings[ATTRIBUTE_PSO_APPLIES_TO].append(dn.toUtf8());
    }

    return settings;
}

QHash<QString, QList<QString> > PSOEditWidget::pso_settings_string_values() {
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

QStringList PSOEditWidget::applied_dn_list() const {
    return dn_applied_list;
}

QLineEdit *PSOEditWidget::name_line_edit() {
    return ui->name_edit;
}

bool PSOEditWidget::settings_are_default() {
    auto current_values = pso_settings_values();
    const QStringList excluded_attrs = {
        ATTRIBUTE_CN,
        ATTRIBUTE_MS_DS_PASSWORD_SETTINGS_PRECEDENCE,
        ATTRIBUTE_APPLIES_TO
    };
    for (const QString &attr : default_setting_values.keys()) {
        if (excluded_attrs.contains(attr)) {
            continue;
        }

        if (default_setting_values[attr] != current_values[attr]) {
            return false;
        }
    }

    return true;
}

void PSOEditWidget::update_defaults() {
    // TODO: Get defaults from Default Domain Policy.

    ui->min_passwd_len_spinbox->setValue(7);
    ui->history_length_spinbox->setValue(24);
    ui->logon_attempts_spinbox->setValue(0);

    ui->lockout_duration_spinbox->setValue(30);
    ui->reset_lockout_spinbox->setValue(30);
    ui->min_age_spinbox->setValue(1);
    ui->max_age_spinbox->setValue(42);

    ui->complexity_req_checkbox->setChecked(true);
    ui->store_passwd_checkbox->setChecked(false);

    ui->applied_list_widget->clear();
}

void PSOEditWidget::on_add() {
    auto dialog = new SelectObjectDialog({CLASS_USER, CLASS_GROUP}, SelectObjectDialogMultiSelection_Yes, this);
    dialog->setWindowTitle(tr("Add applied users/group"));
    dialog->open();

    connect(dialog, &SelectObjectDialog::accepted, this, [this, dialog]() {
       for (auto selected_data : dialog->get_selected_advanced()) {
           QListWidgetItem *item = new QListWidgetItem(g_icon_manager->get_object_icon(dn_get_name(selected_data.category)),
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
    using namespace std::chrono;

    qint64 hundred_nanos = -obj.get_value(attribute).toLongLong();
    milliseconds msecs(hundred_nanos / MILLIS_TO_100_NANOS);

    if (attribute == ATTRIBUTE_MS_DS_LOCKOUT_OBSERVATION_WINDOW || attribute == ATTRIBUTE_MS_DS_LOCKOUT_DURATION) {
        int mins = duration_cast<minutes>(msecs).count();
        return mins;
    }

    if (attribute == ATTRIBUTE_MS_DS_MIN_PASSWORD_AGE || attribute == ATTRIBUTE_MS_DS_MAX_PASSWORD_AGE) {
        int days = duration_cast<hours>(msecs).count() / 24;
        return days;
    }

    return 0;
}
