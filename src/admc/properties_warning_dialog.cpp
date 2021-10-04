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

#include "properties_warning_dialog.h"
#include "ui_properties_warning_dialog.h"

PropertiesWarningDialog::PropertiesWarningDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::PropertiesWarningDialog();
    ui->setupUi(this);

    ui->button_box->addButton(tr("Apply current changes"), QDialogButtonBox::AcceptRole);
    ui->button_box->addButton(tr("Discard changes"), QDialogButtonBox::RejectRole);
}

void PropertiesWarningDialog::set_type(const PropertiesWarningType type) {
    const QString label_text = [&]() {
        switch (type) {
            case PropertiesWarningType_SwitchToAttributes: return tr("You're switching to attributes tab, while another tab has unapplied changes. Choose to apply or discard those changes.");
            case PropertiesWarningType_SwitchFromAttributes: return tr("You're switching from attributes tab, while it has unapplied changes. Choose to apply or discard those changes.");
        }
        return QString();
    }();

    ui->label->setText(label_text);
}
