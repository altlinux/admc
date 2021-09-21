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

#include "change_dc_dialog.h"
#include "ui_change_dc_dialog.h"

#include "console_impls/object_impl.h"
#include "adldap.h"
#include "settings.h"
#include "utils.h"

ChangeDCDialog::ChangeDCDialog(ConsoleWidget *console_arg)
: QDialog(console_arg) {
    ui = new Ui::ChangeDCDialog();
    ui->setupUi(this);

    console = console_arg;
}

void ChangeDCDialog::open() {
    // Load hosts into list when dialog is opened for the
    // first time
    if (ui->select_listwidget->count() == 0) {
        const QString domain = get_default_domain_from_krb5();
        const QList<QString> host_list = get_domain_hosts(domain, QString());

        for (const QString &host : host_list) {
            ui->select_listwidget->addItem(host);
        }
    }

    // Save state to restore in case dialog is rejected
    // later
    const QList<QWidget *> widgetList = get_widget_list();
    
    for (QWidget *widget : widgetList) {
        QRadioButton *radio_button = qobject_cast<QRadioButton*>(widget);
        QLineEdit *lineedit = qobject_cast<QLineEdit*>(widget);
        QListWidget *listwidget = qobject_cast<QListWidget*>(widget);
        QCheckBox *checkbox = qobject_cast<QCheckBox*>(widget);

        if (radio_button != nullptr) {
            original_state[widget] = radio_button->isChecked();
        } else if (checkbox != nullptr) {
            original_state[widget] = checkbox->isChecked();
        } else if (lineedit != nullptr) {
            original_state[widget] = lineedit->text();
        } else if (listwidget != nullptr) {
            original_state[widget] = listwidget->currentRow();
        }
    }

    QDialog::open();
}

void ChangeDCDialog::accept() {
    const QString selected_dc = [&]() {
        if (ui->select_button->isChecked()) {
            QListWidgetItem *current_item = ui->select_listwidget->currentItem();

            if (current_item == nullptr) {
                return QString();
            } else {
                return current_item->text();
            }
        } else {
            return ui->custom_edit->text();
        }
    }();

    if (selected_dc.isEmpty()) {
        message_box_warning(this, tr("Error"), tr("Select or enter a domain controller."));

        return;
    }
    
    AdInterface::set_dc(selected_dc);

    if (ui->save_this_setting_check->isChecked()) {
        settings_set_variant(SETTING_dc, selected_dc);
    }

    const QModelIndex root = get_object_tree_root(console);
    if (root.isValid()) {
        QStandardItem *root_item = console->get_item(root);
        console_object_load_root_text(root_item);
    }

    QDialog::accept();
}

void ChangeDCDialog::reject() {
    // Restore state
    const QList<QWidget *> widgetList = get_widget_list();
    
    for (QWidget *widget : widgetList) {
        QRadioButton *radio_button = qobject_cast<QRadioButton*>(widget);
        QLineEdit *lineedit = qobject_cast<QLineEdit*>(widget);
        QListWidget *listwidget = qobject_cast<QListWidget*>(widget);
        QCheckBox *checkbox = qobject_cast<QCheckBox*>(widget);

        if (radio_button != nullptr) {
            radio_button->setChecked(original_state[widget].toBool());
        } else if (checkbox != nullptr) {
            checkbox->setChecked(original_state[widget].toBool());
        } else if (lineedit != nullptr) {
            lineedit->setText(original_state[widget].toString());
        } else if (listwidget != nullptr) {
            listwidget->setCurrentRow(original_state[widget].toInt());
        }
    }

    QDialog::reject();
}

// NOTE: add any new widgets you add to this list so that
// their state is saved
QList<QWidget *> ChangeDCDialog::get_widget_list() const {
    const QList<QWidget *> out = {
        ui->select_button,
        ui->select_listwidget,
        ui->custom_button,
        ui->custom_edit,
        ui->save_this_setting_check,
    };

    return out;
}
