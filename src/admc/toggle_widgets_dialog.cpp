/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "toggle_widgets_dialog.h"

#include "settings.h"

#include <QString>
#include <QFormLayout>
#include <QCheckBox>
#include <QDialogButtonBox>

ToggleWidgetsDialog::ToggleWidgetsDialog(QWidget *parent)
: QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Toggle widgets"));

    auto checks_layout = new QFormLayout();

    auto add =
    [checks_layout](const QString &label, const BoolSetting setting) {
        auto checkbox = new QCheckBox();
        checks_layout->addRow(label, checkbox);
        SETTINGS()->connect_checkbox_to_bool_setting(checkbox, setting);
    };

    add(tr("Directory tree"), BoolSetting_ShowContainers);
    add(tr("Status Log"), BoolSetting_ShowStatusLog);
    add(tr("Contents header"), BoolSetting_ShowContentsHeader);

    auto buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok);
    connect(
        buttonbox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addLayout(checks_layout);
    layout->addWidget(buttonbox);
}
