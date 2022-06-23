/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#include "find_policy_dialog.h"
#include "ui_find_policy_dialog.h"

#include "adldap.h"
#include "settings.h"

enum SearchItem {
    SearchItem_Name,
    SearchItem_GUID,
};

// TODO: "not contains" item for condition combo. Need to
// add Condition_NotContains to ad_filter.

FindPolicyDialog::FindPolicyDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::FindPolicyDialog();
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    // Fill search item combo
    const QList<SearchItem> search_item_list = {
        SearchItem_Name,
        SearchItem_GUID,
    };

    for (const SearchItem &search_item : search_item_list) {
        const QString search_item_string = [&]() {
            switch (search_item) {
                case SearchItem_Name: return tr("Name");
                case SearchItem_GUID: return tr("GUID");
            }

            return QString();
        }();

        ui->search_item_combo->addItem(search_item_string, (int) search_item);
    }

    // Fill condition combo
    const QList<Condition> condition_list = {
        Condition_Contains,
        Condition_Equals,
        Condition_StartsWith,
        Condition_EndsWith,
    };

    for (const Condition &condition : condition_list) {
        const QString condition_string = condition_to_display_string(condition);

        ui->condition_combo->addItem(condition_string, (int) condition);
    }

    settings_setup_dialog_geometry(SETTING_find_policy_dialog_geometry, this);
}

FindPolicyDialog::~FindPolicyDialog() {
    delete ui;
}
