/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include "ui/dialog/properties_warning.h"
#include "ui/dialog/ui_properties_warning.h"

#include <QPushButton>

PropertiesWarningDialog::PropertiesWarningDialog(const PropertiesWarningType type, QWidget *parent)
: QDialog(parent) {
    ui = new Ui::PropertiesWarningDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    QString label_text;
    switch (type) {
    case PropertiesWarningType_SwitchToAttributes:
        label_text =
            tr("You're switching to attributes tab, while another tab has unapplied changes. Choose to apply or discard those changes.");
        break;
    case PropertiesWarningType_SwitchFromAttributes:
        label_text =
            tr("You're switching from attributes tab, while it has unapplied changes. Choose to apply or discard those changes.");
        break;
    default:
        label_text = QString();
    }

    ui->label->setText(label_text);

    auto apply_button = ui->button_box->button(QDialogButtonBox::Apply);
    auto discard_button = ui->button_box->button(QDialogButtonBox::Discard);

    connect(
        apply_button, &QPushButton::clicked,
        this, &PropertiesWarningDialog::on_apply_button);
    connect(
        discard_button, &QPushButton::clicked,
        this, &PropertiesWarningDialog::on_discard_button);
}

PropertiesWarningDialog::~PropertiesWarningDialog() {
    delete ui;
}

void PropertiesWarningDialog::on_apply_button() {
    emit applied();

    QDialog::accept();
}

void PropertiesWarningDialog::on_discard_button() {
    emit discarded();

    QDialog::accept();
}

void PropertiesWarningDialog::retranslate_ui() {
    ui->retranslateUi(this);
    for (auto* widget : children()) {
        QEvent languageEvent(QEvent::LanguageChange);
        QCoreApplication::sendEvent(widget, &languageEvent);
    }
}

bool PropertiesWarningDialog::event(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    return QDialog::event(event);
}

