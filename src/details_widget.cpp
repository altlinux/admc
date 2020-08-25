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

#include "details_widget.h"
#include "details_tab.h"
#include "attributes_tab.h"
#include "members_tab.h"
#include "account_tab.h"
#include "general_tab.h"
#include "address_tab.h"
#include "object_tab.h"
#include "ad_interface.h"
#include "settings.h"
#include "object_context_menu.h"
#include "containers_widget.h"
#include "contents_widget.h"
#include "status.h"
#include "utils.h"

#include <QAction>
#include <QTabWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>

DetailsWidget::DetailsWidget(ObjectContextMenu *object_context_menu, ContainersWidget *containers_widget, ContentsWidget *contents_widget, QWidget *parent)
: QWidget(parent)
{
    title_label = new QLabel(this);
    tab_widget = new QTabWidget(this);

    tabs[TabHandle_General] = new GeneralTab(this);
    tabs[TabHandle_Object] = new ObjectTab(this);
    tabs[TabHandle_Attributes] = new AttributesTab(this);
    tabs[TabHandle_Account] = new AccountTab(this);
    tabs[TabHandle_Members] = new MembersTab(object_context_menu, this);
    tabs[TabHandle_Address] = new AddressTab(this);

    // NOTE: need to add all tabs so that tab widget gains ownership of them
    for (auto tab : tabs) {
        tab_widget->addTab(tab, "");
    }

    button_box = new QDialogButtonBox(QDialogButtonBox::Apply |  QDialogButtonBox::Cancel, this);

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(title_label);
    layout->addWidget(tab_widget);
    layout->addWidget(button_box);

    connect(
        AdInterface::instance(), &AdInterface::logged_in,
        this, &DetailsWidget::on_logged_in);
    connect(
        AdInterface::instance(), &AdInterface::modified,
        this, &DetailsWidget::on_ad_modified);

    connect(
        object_context_menu, &ObjectContextMenu::details,
        this, &DetailsWidget::on_context_menu_details);
    connect(
        containers_widget, &ContainersWidget::clicked_dn,
        this, &DetailsWidget::on_containers_clicked_dn);
    connect(
        contents_widget, &ContentsWidget::clicked_dn,
        this, &DetailsWidget::on_contents_clicked_dn);
    connect(
        button_box->button(QDialogButtonBox::Apply), &QPushButton::clicked,
        this, &DetailsWidget::on_apply);
    connect(
        button_box->button(QDialogButtonBox::Cancel), &QPushButton::clicked,
        this, &DetailsWidget::on_cancel);

    reload("");
}

QString DetailsWidget::get_target() const {
    return target;
}

void DetailsWidget::tab_edited(DetailsTab *tab) {
    // Add asterisk to end of tab text to indicate that it was edited
    const int tab_index = tab_widget->indexOf(tab);
    if (tab_index != -1) {
        const QString current_text = tab_widget->tabText(tab_index);
        const QString new_text = set_edited_marker(current_text, tab->changed());
        tab_widget->setTabText(tab_index, new_text);
    }

    // Enable/disable apply and cancel depending on changed state
    bool any_changed = false;
    for (auto t : tabs) {
        if (t->accepts_target() && t->changed()) {
            any_changed = true;
        }
    }

    button_box->setEnabled(any_changed);
}

void DetailsWidget::reload(const QString &new_target) {
    target = new_target;
    
    const QString name = AdInterface::instance()->attribute_get(target, ATTRIBUTE_NAME);
    const QString title_text = name.isEmpty() ? tr("Details") : QString(tr("%1 Details")).arg(name);
    title_label->setText(title_text);

    // Save current tab/index to restore later
    QWidget *old_tab = tab_widget->widget(tab_widget->currentIndex());

    // Setup tabs
    tab_widget->clear();

    for (int i = 0; i < TabHandle_COUNT; i++) {
        const TabHandle tab_handle = (TabHandle) i;
        DetailsTab *tab = tabs[i];
        const bool accepts_target = tab->accepts_target();

        if (accepts_target) {
            tab->reload();

            auto get_tab_text =
            [tab_handle]() -> QString {
                switch (tab_handle) {
                    case TabHandle_General: return tr("General");
                    case TabHandle_Object: return tr("Object");
                    case TabHandle_Attributes: return tr("Attributes");
                    case TabHandle_Account: return tr("Account");
                    case TabHandle_Members: return tr("Members");
                    case TabHandle_Address: return tr("Address");
                    case TabHandle_COUNT: return tr("COUNT"); 
                }
                return "";
            };
            const QString tab_text = get_tab_text();

            tab_widget->addTab(tab, tab_text);
        }
    }

    // Restore current index if it is still shown
    // Otherwise current index is set to first tab by default
    const int old_tab_index_in_new_tabs = tab_widget->indexOf(old_tab);
    if (old_tab_index_in_new_tabs != -1) {
        tab_widget->setCurrentIndex(old_tab_index_in_new_tabs);
    }

    if (tab_widget->count() > 0) {
        button_box->show();
    } else {
        button_box->hide();
    }

    // Disable apply/cancel since this is a fresh reload and there are no changes
    button_box->setEnabled(false);
}

void DetailsWidget::on_logged_in() {
    // Clear data on new login
    reload("");
}

void DetailsWidget::on_ad_modified() {
    reload(target);
}

void DetailsWidget::on_apply() {
    const int errors_index = Status::instance()->get_errors_size();

    bool all_verified = true;
    for (auto tab : tabs) {
        if (tab->accepts_target()) {
            const bool verify_success = tab->verify();
            if (!verify_success) {
                all_verified = false;
            }
        }
    }

    if (all_verified) {
        AdInterface::instance()->start_batch();
        for (auto tab : tabs) {
            if (tab->accepts_target()) {
                tab->apply();
            }
        }
        AdInterface::instance()->end_batch();

        Status::instance()->show_errors_popup(errors_index);
    }
}

void DetailsWidget::on_cancel() {
    reload(target);
}

void DetailsWidget::on_containers_clicked_dn(const QString &dn) {
    const bool details_from_containers = Settings::instance()->get_bool(BoolSetting_DetailsFromContainers);
    if (details_from_containers) {
        reload(dn);
    }
}

void DetailsWidget::on_contents_clicked_dn(const QString &dn) {
    const bool details_from_contents = Settings::instance()->get_bool(BoolSetting_DetailsFromContents);
    if (details_from_contents) {
        reload(dn);
    }
}

void DetailsWidget::on_context_menu_details(const QString &dn) {
    reload(dn);
}
