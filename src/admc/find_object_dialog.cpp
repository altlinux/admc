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

#include "find_object_dialog.h"

#include "ad_config.h"
#include "find_results.h"
#include "find_widget.h"
#include "globals.h"
#include "settings.h"
#include "console_widget/console_widget.h"

#include <QMenuBar>
#include <QVBoxLayout>

FindObjectDialog::FindObjectDialog(const QList<QString> classes, const QString default_base, QWidget *parent)
: QDialog(parent) {
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Find Objects"));

    auto menubar = new QMenuBar();
    auto action_menu = menubar->addMenu(tr("&Action"));
    auto view_menu = menubar->addMenu(tr("&View"));

    auto find_widget = new FindWidget(classes, default_base);

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setMenuBar(menubar);
    layout->addWidget(find_widget);

    ConsoleWidget *console = find_widget->find_results->console;
    console->add_actions(action_menu);

    view_menu->addAction(console->customize_columns_action());

    settings_setup_dialog_geometry(SETTING_find_object_dialog_geometry, this);

    connect(
        action_menu, &QMenu::aboutToShow,
        console, &ConsoleWidget::update_actions);
}
