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
#include "find_policy_dialog_p.h"
#include "ui_find_policy_dialog.h"

#include "adldap.h"
#include "settings.h"

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

    ui->console->set_scope_view_visible(false);

    settings_setup_dialog_geometry(SETTING_find_policy_dialog_geometry, this);

    const QVariant console_state = settings_get_variant(SETTING_find_policy_dialog_console_state);
    ui->console->restore_state(console_state);

    connect(
        ui->add_button, &QAbstractButton::clicked,
        this, &FindPolicyDialog::add_filter);
}

FindPolicyDialog::~FindPolicyDialog() {
    const QVariant console_state = ui->console->save_state();
    settings_set_variant(SETTING_find_policy_dialog_console_state, console_state);

    delete ui;
}

void FindPolicyDialog::add_filter() {
    const QString filter = [&]() {
        const QString attribute = [&]() -> QString {
            const SearchItem search_item = (SearchItem) ui->search_item_combo->currentData().toInt();
            switch (search_item) {
                case SearchItem_Name: return ATTRIBUTE_DISPLAY_NAME;
                case SearchItem_GUID: return ATTRIBUTE_CN;
            }

            return QString();
        }();

        const Condition condition = (Condition) ui->condition_combo->currentData().toInt();

        const QString value = ui->value_edit->text();

        const QString out = filter_CONDITION(condition, attribute, value);

        return out;
    }();

    const QString filter_display = [&]() {
        const QString search_item_display = [&]() -> QString {
            const SearchItem search_item = (SearchItem) ui->search_item_combo->currentData().toInt();
            switch (search_item) {
                case SearchItem_Name: return tr("Name");
                case SearchItem_GUID: return tr("GUID");
            }

            return QString();
        }();

        const QString condition_display = [&]() {
            const Condition condition = (Condition) ui->condition_combo->currentData().toInt();
            const QString out = condition_to_display_string(condition);

            return out;
        }();

        const QString value = ui->value_edit->text();

        const QString out = QString("%1 %2: \"%3\"").arg(search_item_display, condition_display, value);

        return out;
    }();

    auto item = new QListWidgetItem();
    item->setText(filter_display);
    item->setData(Qt::UserRole, filter);
    ui->filter_list->addItem(item);

    ui->value_edit->clear();
}
