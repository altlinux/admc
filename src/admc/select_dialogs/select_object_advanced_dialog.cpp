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

#include "select_object_advanced_dialog.h"
#include "ui_select_object_advanced_dialog.h"

#include "adldap.h"
#include "globals.h"
#include "settings.h"

#include <QMenuBar>

SelectObjectAdvancedDialog::SelectObjectAdvancedDialog(const QList<QString> classes, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::SelectObjectAdvancedDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    auto menubar = new QMenuBar();
    layout()->setMenuBar(menubar);
    auto view_menu = menubar->addMenu(tr("&View"));

    ui->find_widget->set_classes(classes, classes);
    ui->find_widget->setup_view_menu(view_menu);

    const QVariant console_state = settings_get_variant(SETTING_select_object_advanced_dialog_console_state);
    ui->find_widget->restore_console_state(console_state);

    settings_setup_dialog_geometry(SETTING_select_object_advanced_dialog_geometry, this);
}

SelectObjectAdvancedDialog::~SelectObjectAdvancedDialog() {
    const QVariant console_state = ui->find_widget->save_console_state();
    settings_set_variant(SETTING_select_object_advanced_dialog_console_state, console_state);

    delete ui;
}

QList<QString> SelectObjectAdvancedDialog::get_selected_dns() const {
    return ui->find_widget->get_selected_dns();
}
