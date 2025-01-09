/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#include "tabs/attributes_tab_filter_menu.h"

#include "adldap.h"
#include "attribute_dialogs/bool_attribute_dialog.h"
#include "attribute_dialogs/datetime_attribute_dialog.h"
#include "attribute_dialogs/list_attribute_dialog.h"
#include "attribute_dialogs/octet_attribute_dialog.h"
#include "attribute_dialogs/string_attribute_dialog.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QStandardItemModel>

AttributesTabFilterMenu::AttributesTabFilterMenu(QWidget *parent)
: QMenu(parent) {
    const QList<QVariant> state = settings_get_variant(SETTING_attributes_tab_filter_state).toList();

    auto add_filter_action = [&](const QString text, const AttributeFilter filter) {
        QAction *action = addAction(text);
        action->setText(text);
        action->setObjectName(QString::number(filter));
        action->setCheckable(true);

        const bool is_checked = [&]() {
            if (filter < state.size()) {
                return state[filter].toBool();
            } else {
                return true;
            }
        }();
        action->setChecked(is_checked);

        action_map.insert(filter, action);

        connect(
            action, &QAction::toggled,
            this, &AttributesTabFilterMenu::filter_changed);
    };

    add_filter_action(tr("Unset"), AttributeFilter_Unset);
    add_filter_action(tr("Read-only"), AttributeFilter_ReadOnly);

    addSeparator();

    add_filter_action(tr("Mandatory"), AttributeFilter_Mandatory);
    add_filter_action(tr("Optional"), AttributeFilter_Optional);

    addSeparator();

    add_filter_action(tr("System-only"), AttributeFilter_SystemOnly);
    add_filter_action(tr("Constructed"), AttributeFilter_Constructed);
    add_filter_action(tr("Backlink"), AttributeFilter_Backlink);

    connect(
        action_map[AttributeFilter_ReadOnly], &QAction::toggled,
        this, &AttributesTabFilterMenu::on_read_only_changed);
    on_read_only_changed();
}

AttributesTabFilterMenu::~AttributesTabFilterMenu() {
    const QList<QVariant> state = [&]() {
        QList<QVariant> out;

        for (int fitler_i = 0; fitler_i < AttributeFilter_COUNT; fitler_i++) {
            const AttributeFilter filter = (AttributeFilter) fitler_i;
            const QAction *action = action_map[filter];
            const QVariant filter_state = QVariant(action->isChecked());

            out.append(filter_state);
        }

        return out;
    }();

    settings_set_variant(SETTING_attributes_tab_filter_state, state);
}

void AttributesTabFilterMenu::on_read_only_changed() {
    const bool read_only_is_enabled = action_map[AttributeFilter_ReadOnly]->isChecked();

    const QList<AttributeFilter> read_only_sub_filters = {
        AttributeFilter_SystemOnly,
        AttributeFilter_Constructed,
        AttributeFilter_Backlink,
    };

    for (const AttributeFilter &filter : read_only_sub_filters) {
        action_map[filter]->setEnabled(read_only_is_enabled);

        // Turning off read only turns off the sub read only
        // filters. Note that turning ON read only doesn't do
        // the opposite.
        if (!read_only_is_enabled) {
            action_map[filter]->setChecked(false);
        }
    }
}

bool AttributesTabFilterMenu::filter_is_enabled(const AttributeFilter filter) const {
    return action_map[filter]->isChecked();
}
