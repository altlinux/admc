/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2023 BaseALT Ltd.
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

#include "time_span_attribute_dialog.h"
#include "ui_time_span_attribute_dialog.h"

#include "settings.h"
#include "utils.h"
#include "ad_display.cpp"
#include "ad_defines.h"

#include <QString>

TimeSpanAttributeDialog::TimeSpanAttributeDialog(const QList<QByteArray> &value_list, const QString &attribute, const bool read_only, QWidget *parent) :
    AttributeDialog(attribute, read_only, parent),
    ui(new Ui::TimeSpanAttributeDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    AttributeDialog::load_attribute_label(ui->attribute_label);

    const QByteArray value = value_list.value(0, QByteArray());

    set_line_edit_to_time_span_format(ui->time_span_edit);

    ui->time_span_edit->setReadOnly(read_only);
    ui->time_span_edit->setText(timespan_display_value(value));

    settings_setup_dialog_geometry(SETTING_time_span_attribute_dialog_geometry, this);
}

TimeSpanAttributeDialog::~TimeSpanAttributeDialog()
{
    delete ui;
}

QList<QByteArray> TimeSpanAttributeDialog::get_value_list() const
{
    const QString new_value_string = ui->time_span_edit->text();
    QByteArray value = QByteArray::number(0);

    if (new_value_string == "(none)") {
        return {value};
    }

    if (new_value_string == "(never)") {
        value = QByteArray::number(LLONG_MIN);
        return {value};
    }

    QStringList d_hh_mm_ss = new_value_string.split(':');
    if (d_hh_mm_ss.size() != 4) {
        return {value};
    }

    qint64 hundred_nanos = 100 * std::chrono::nanoseconds(-d_hh_mm_ss[0].toLongLong() * DAYS_TO_SECONDS +
                                                          -d_hh_mm_ss[1].toLongLong() * HOURS_TO_SECONDS +
                                                          -d_hh_mm_ss[2].toLongLong() * MINUTES_TO_SECONDS
                                                          -d_hh_mm_ss[3].toLongLong()).count();
    value = QByteArray::number(hundred_nanos);
    return {value};
}
