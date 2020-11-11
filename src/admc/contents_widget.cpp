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

#include "contents_widget.h"
#include "containers_widget.h"
#include "settings.h"
#include "ad_interface.h"
#include "object_list_widget.h"

#include <QVBoxLayout>

ContentsWidget::ContentsWidget(ContainersWidget *containers_widget)
: QWidget()
{   
    object_list = new ObjectListWidget();

    const auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(object_list);

    connect(
        containers_widget, &ContainersWidget::selected_changed,
        this, &ContentsWidget::on_containers_selected_changed);
    connect(
        AD(), &AdInterface::modified,
        this, &ContentsWidget::on_ad_modified);

    const BoolSettingSignal *advanced_view_setting = SETTINGS()->get_bool_signal(BoolSetting_AdvancedView);
    connect(
        advanced_view_setting, &BoolSettingSignal::changed,
        [this]() {
            change_target(target_dn);
        });
}

void ContentsWidget::on_containers_selected_changed(const QString &dn) {
    change_target(dn);
}

void ContentsWidget::on_ad_modified() {
    change_target(target_dn);
}

void ContentsWidget::change_target(const QString &dn) {
    target_dn = dn;

    object_list->load_children(target_dn);
}
