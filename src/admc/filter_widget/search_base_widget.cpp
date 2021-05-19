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

#include "filter_widget/search_base_widget.h"

#include "adldap.h"
#include "globals.h"
#include "select_container_dialog.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QtWidgets>

// TODO: missing "Entire directory" in search base combo. Not 100% sure what it's supposed to be, the tippy-top domain? Definitely need it for work with multiple domains.

SearchBaseWidget::SearchBaseWidget(const QString &default_search_base)
: QWidget()
{
    const QString domain_head = g_adconfig->domain_head();

    combo = new QComboBox();

    auto add_search_base_to_combo =
    [this](const QString dn) {
        const QString name = dn_get_name(dn);
        combo->addItem(name, dn);
    };

    if (default_search_base == domain_head || default_search_base.isEmpty()) {
        add_search_base_to_combo(domain_head);
    } else {
        add_search_base_to_combo(domain_head);
        add_search_base_to_combo(default_search_base);
    }

    auto browse_button = new QPushButton(tr("Browse"));
    browse_button->setAutoDefault(false);

    const int last_index = combo->count() - 1;
    combo->setCurrentIndex(last_index);

    auto layout = new QHBoxLayout();
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(combo);
    layout->addWidget(browse_button);

    connect(
        browse_button, &QAbstractButton::clicked,
        this, &SearchBaseWidget::browse);
}

QString SearchBaseWidget::get_search_base() const {
    const int index = combo->currentIndex();
    const QVariant item_data = combo->itemData(index);

    return item_data.toString();
}

void SearchBaseWidget::browse() {
    auto dialog = new SelectContainerDialog(this);

    connect(
        dialog, &SelectContainerDialog::accepted,
        [this, dialog]() {
            const QString selected = dialog->get_selected();
            const QString name = dn_get_name(selected);

            combo->addItem(name, selected);

            // Select newly added search base in combobox
            const int new_base_index = combo->count() - 1;
            combo->setCurrentIndex(new_base_index);
        });

    dialog->open();
}

void SearchBaseWidget::serialize(QDataStream &stream) const {
    QList<QString> name_list;
    QList<QString> dn_list;

    for (int i = 0; i < combo->count(); i++) {
        const QString dn = combo->itemData(i).toString();
        const QString name = combo->itemText(i);
        name_list.append(name);
        dn_list.append(dn);
    }

    const int current_index = combo->currentIndex();

    stream << dn_list;
    stream << name_list;
    stream << current_index;
}

void SearchBaseWidget::deserialize(QDataStream &stream) {
    QList<QString> name_list;
    QList<QString> dn_list;
    int current_index;

    stream >> dn_list;
    stream >> name_list;
    stream >> current_index;

    combo->clear();

    for (int i = 0; i < dn_list.size(); i++) {
        const QString dn = dn_list[i];
        const QString name = name_list[i];
        combo->addItem(name, dn);
    }

    combo->setCurrentIndex(current_index);
}

QDataStream &operator<<(QDataStream &stream, const SearchBaseWidget *widget) {
    widget->serialize(stream);

    return stream;
}

QDataStream &operator>>(QDataStream &stream, SearchBaseWidget *widget) {
    widget->deserialize(stream);
    
    return stream;
}
