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

#include "tabs/general_other_tab.h"
#include "tabs/ui_general_other_tab.h"

#include "adldap.h"
#include "attribute_edits/string_edit.h"
#include "tabs/general_other_tab.h"

GeneralOtherTab::GeneralOtherTab(const AdObject &object, QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralOtherTab();
    ui->setupUi(this);

    load_name_label(ui->name_label, object);

    new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, edit_list, this);
}

GeneralOtherTab::~GeneralOtherTab() {
    delete ui;
}

void load_name_label(QLabel *name_label, const AdObject &object) {
    const QString name = object.get_string(ATTRIBUTE_NAME);
    name_label->setText(name);

    if (object.is_empty()) {
        name_label->setText(QCoreApplication::translate("general_other_tab.cpp", "Failed to load object information. Check your connection."));
    }
}
