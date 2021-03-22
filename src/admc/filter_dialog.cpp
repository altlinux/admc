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

#include "filter_dialog.h"
#include "filter.h"
#include "ad/ad_config.h"

#include "filter_widget/filter_widget.h"

#include <QDialogButtonBox>
#include <QVBoxLayout>

FilterDialog::FilterDialog(QWidget *parent)
: QDialog(parent)
{   
    setWindowTitle(tr("Filter contents"));

    // NOTE: Can't filter out container objects, because otherwise the whole tree can't be displayed, so only allow filtering by non-container classes.
    // = all classes - container classes
    const QList<QString> noncontainer_classes =
    []() {
        QList<QString> out = filter_classes;

        const QList<QString> container_classes = ADCONFIG()->get_filter_containers();
        for (const QString &container_class : container_classes) {
            out.removeAll(container_class);
        }

        return out;
    }();

    filter_widget = new FilterWidget(noncontainer_classes);

    auto buttonbox = new QDialogButtonBox();
    buttonbox->addButton(QDialogButtonBox::Ok);
    buttonbox->addButton(QDialogButtonBox::Cancel);

    connect(
        buttonbox, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        buttonbox, &QDialogButtonBox::rejected,
        this, &QDialog::reject);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(filter_widget);
    layout->addWidget(buttonbox);
}
