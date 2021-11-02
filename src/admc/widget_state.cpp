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

#include "widget_state.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QRadioButton>

class WidgetStatePrivate final {
public:
    QList<QWidget *> widget_list;
    QHash<QWidget *, QVariant> state;
};

WidgetState::WidgetState() {
    d = new WidgetStatePrivate();
}

WidgetState::~WidgetState() {
    delete d;
}

void WidgetState::set_widget_list(const QList<QWidget *> &widget_list) {
    d->widget_list = widget_list;

    // Clear any previous state since it is now invalid
    d->state.clear();
}

void WidgetState::save() {
    d->state.clear();

    for (QWidget *widget : d->widget_list) {
        QRadioButton *radio_button = qobject_cast<QRadioButton *>(widget);
        QLineEdit *line_edit = qobject_cast<QLineEdit *>(widget);
        QListWidget *list_widget = qobject_cast<QListWidget *>(widget);
        QCheckBox *check_box = qobject_cast<QCheckBox *>(widget);
        QComboBox *combo = qobject_cast<QComboBox *>(widget);

        if (radio_button != nullptr) {
            d->state[widget] = radio_button->isChecked();
        } else if (check_box != nullptr) {
            d->state[widget] = check_box->isChecked();
        } else if (line_edit != nullptr) {
            d->state[widget] = line_edit->text();
        } else if (list_widget != nullptr) {
            d->state[widget] = list_widget->currentRow();
        } else if (combo != nullptr) {
            d->state[widget] = combo->currentIndex();
        }
    }
}

void WidgetState::restore() {
    for (QWidget *widget : d->widget_list) {
        QRadioButton *radio_button = qobject_cast<QRadioButton *>(widget);
        QLineEdit *line_edit = qobject_cast<QLineEdit *>(widget);
        QListWidget *list_widget = qobject_cast<QListWidget *>(widget);
        QCheckBox *check_box = qobject_cast<QCheckBox *>(widget);
        QComboBox *combo = qobject_cast<QComboBox *>(widget);

        if (radio_button != nullptr) {
            radio_button->setChecked(d->state[widget].toBool());
        } else if (check_box != nullptr) {
            check_box->setChecked(d->state[widget].toBool());
        } else if (line_edit != nullptr) {
            line_edit->setText(d->state[widget].toString());
        } else if (list_widget != nullptr) {
            list_widget->setCurrentRow(d->state[widget].toInt());
        } else if (combo != nullptr) {
            combo->setCurrentIndex(d->state[widget].toInt());
        }
    }
}
