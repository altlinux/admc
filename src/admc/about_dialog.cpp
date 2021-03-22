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

#include "about_dialog.h"

#include "config.h"

#include <QLabel>
#include <QString>
#include <QDialogButtonBox>
#include <QVBoxLayout>

AboutDialog::AboutDialog(QWidget *parent)
: QDialog(parent)
{
    auto version_label = new QLabel(QString(tr("Version %1")).arg(ADMC_VERSION));
    version_label->setAlignment(Qt::AlignHCenter);

    auto description_label = new QLabel(tr("ADMC is a tool for Active Directory administration."));

    auto license_label = new QLabel(tr("Copyright (C) 2020 BaseALT Ltd."));

    auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(version_label);
    layout->addWidget(description_label);
    layout->addWidget(license_label);
    layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
}
