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

#include "edits/expiry_widget.h"
#include "edits/ui_expiry_widget.h"

#include "adldap.h"
#include "globals.h"

#include <QButtonGroup>

// TODO: do end of day by local time or UTC????

const QTime END_OF_DAY(23, 59);

ExpiryWidget::ExpiryWidget(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::ExpiryWidget();
    ui->setupUi(this);

    ui->never_check->setAutoExclusive(true);
    ui->end_of_check->setAutoExclusive(true);

    auto button_group = new QButtonGroup(this);
    button_group->addButton(ui->never_check);
    button_group->addButton(ui->end_of_check);

    auto date_layout = new QHBoxLayout();
    date_layout->addWidget(ui->end_of_check);
    date_layout->addWidget(ui->date_edit);

    connect(
        ui->never_check, &QCheckBox::stateChanged,
        this, &ExpiryWidget::on_never_check);
    connect(
        ui->end_of_check, &QCheckBox::stateChanged,
        this, &ExpiryWidget::on_end_of_check);
    connect(
        ui->date_edit, &QDateEdit::dateChanged,
        this, &ExpiryWidget::edited);
}

void ExpiryWidget::load(const AdObject &object) {
    const bool never = [object]() {
        const QString expiry_string = object.get_string(ATTRIBUTE_ACCOUNT_EXPIRES);
        return large_integer_datetime_is_never(expiry_string);
    }();

    if (never) {
        ui->never_check->setChecked(true);

        ui->end_of_check->setChecked(false);
        ui->date_edit->setEnabled(false);
    } else {
        ui->never_check->setChecked(false);

        ui->end_of_check->setChecked(true);
        ui->date_edit->setEnabled(true);
    }

    const QDate date = [=]() {
        if (never) {
            // Default to current date when expiry is never
            return QDate::currentDate();
        } else {
            const QDateTime datetime = object.get_datetime(ATTRIBUTE_ACCOUNT_EXPIRES, g_adconfig);
            return datetime.date();
        }
    }();

    ui->date_edit->setDate(date);
}

void ExpiryWidget::set_read_only(const bool read_only) {
    ui->never_check->setDisabled(read_only);
    ui->end_of_check->setDisabled(read_only);
    ui->date_edit->setReadOnly(read_only);
}

bool ExpiryWidget::apply(AdInterface &ad, const QString &dn) const {
    const bool never = ui->never_check->isChecked();

    if (never) {
        return ad.attribute_replace_string(dn, ATTRIBUTE_ACCOUNT_EXPIRES, AD_LARGE_INTEGER_DATETIME_NEVER_2);
    } else {
        const QDateTime datetime = QDateTime(ui->date_edit->date(), END_OF_DAY, Qt::UTC);

        return ad.attribute_replace_datetime(dn, ATTRIBUTE_ACCOUNT_EXPIRES, datetime);
    }
}

void ExpiryWidget::on_never_check() {
    if (ui->never_check->isChecked()) {
        ui->date_edit->setEnabled(false);

        emit edited();
    }
}

void ExpiryWidget::on_end_of_check() {
    if (ui->end_of_check->isChecked()) {
        ui->date_edit->setEnabled(true);

        emit edited();
    }
}
