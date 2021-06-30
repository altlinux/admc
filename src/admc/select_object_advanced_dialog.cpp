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

#include "select_object_advanced_dialog.h"

#include "adldap.h"
#include "find_widget.h"
#include "globals.h"

#include <QDialogButtonBox>
#include <QList>
#include <QVBoxLayout>

SelectObjectAdvancedDialog::SelectObjectAdvancedDialog(const QList<QString> classes, QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Find and select objects"));

    find_widget = new FindWidget(classes, g_adconfig->domain_head());

    auto buttons = new QDialogButtonBox();
    buttons->addButton(QDialogButtonBox::Ok);
    buttons->addButton(QDialogButtonBox::Cancel);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(find_widget);
    layout->addWidget(buttons);

    connect(
        buttons, &QDialogButtonBox::accepted,
        this, &SelectObjectAdvancedDialog::accept);
    connect(
        buttons, &QDialogButtonBox::rejected,
        this, &SelectObjectAdvancedDialog::reject);
}

QList<QList<QStandardItem *>> SelectObjectAdvancedDialog::get_selected_rows() const {
    return find_widget->get_selected_rows();
}
