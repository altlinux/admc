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

#include "filter_widget.h"
#include "ad_interface.h"

#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QComboBox>

const QHash<QString, QList<QString>> attributes_by_class = {
    {CLASS_USER, {
        ATTRIBUTE_FIRST_NAME,
        ATTRIBUTE_LAST_NAME,
    }},
    {CLASS_GROUP, {
        ATTRIBUTE_DESCRIPTION,
        ATTRIBUTE_SAMACCOUNT_NAME,
    }},
}; 

FilterWidget::FilterWidget()
: QWidget()
{
    advanced_tab = new QWidget();
    {
        ldap_filter_edit = new QPlainTextEdit(this);

        auto layout = new QVBoxLayout();
        advanced_tab->setLayout(layout);
        layout->addWidget(ldap_filter_edit);
    }

    normal_tab = new QWidget();
    {
        class_combo = new QComboBox();
        class_combo->addItem(tr("User"), CLASS_USER);
        class_combo->addItem(tr("Group"), CLASS_GROUP);

        // TODO: when class combo changes, reload attributes combo

        attributes_combo = new QComboBox();

        // TODO: when i got the list of attributes, insert attribute names as values, use display strings for labels

        auto layout = new QGridLayout();
        normal_tab->setLayout(layout);
        layout->addWidget(class_combo, 0, 0);
        layout->addWidget(attributes_combo, 0, 1);
    }

    tab_widget = new QTabWidget();
    tab_widget->addTab(normal_tab, tr("Filter"));
    tab_widget->addTab(advanced_tab, tr("Advanced"));

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(tab_widget);

    connect(
        class_combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &FilterWidget::on_class_combo);
    on_class_combo();

    connect(
        ldap_filter_edit, &QPlainTextEdit::textChanged,
        [this]() {
            emit changed();
        });
}

// Fill attributes combo with attributes for selected class
void FilterWidget::on_class_combo() {
    attributes_combo->clear();
    
    const QList<QString> attributes =
    [this]() {
        const int index = class_combo->currentIndex();
        const QVariant item_data = class_combo->itemData(index);
        const QString object_class = item_data.toString();
        
        return attributes_by_class[object_class];
    }();

    for (const auto attribute : attributes) {
        // TODO: insert attribute display strings
        attributes_combo->addItem(attribute, attribute);
    }
}

QString FilterWidget::get_filter() const {
    const QWidget *current_tab = tab_widget->currentWidget();

    if (current_tab == advanced_tab) {
        return ldap_filter_edit->toPlainText();
    } else {
        return QString();
    }

    return QString();
}
