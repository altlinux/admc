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

#include "filter_custom_dialog.h"

#include "adldap.h"
#include "globals.h"

#include "filter_widget/filter_widget.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

// TODO: implement canceling. Need to be able to load/unload
// filter widget state though. For example, one way to
// implement would be to save old state on open, then reload
// it when cancel is pressed.

FilterCustomDialog::FilterCustomDialog(QWidget *parent)
: QDialog(parent)
{   
    setWindowTitle(tr("Custom filter"));

    // NOTE: Can't filter out container objects, because
    // otherwise the whole tree can't be displayed, so only
    // allow filtering by non-container classes
    const QList<QString> noncontainer_classes = g_adconfig->get_noncontainer_classes();   

    filter_widget = new FilterWidget(noncontainer_classes);

    auto buttonbox = new QDialogButtonBox();
    buttonbox->addButton(QDialogButtonBox::Ok);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(filter_widget);
    layout->addWidget(buttonbox);

    connect(
        buttonbox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
}
