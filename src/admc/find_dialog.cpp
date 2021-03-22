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

#include "find_dialog.h"

#include "find_widget.h"
#include "find_results.h"
#include "ad_config.h"
#include "object_menu.h"
#include "properties_dialog.h"
#include "console_widget/customize_columns_dialog.h"

#include <QList>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMenuBar>
#include <QTreeView>

FindDialog::FindDialog(const QList<QString> classes, const QString default_search_base, QWidget *parent)
: QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Find objects"));

    auto menubar = new QMenuBar();
    auto action_menu = menubar->addMenu(tr("&Action"));
    auto view_menu = menubar->addMenu(tr("&View"));

    auto find_widget = new FindWidget(classes, default_search_base);
    find_widget->find_results->setup_context_menu();

    QTreeView *results_view = find_widget->find_results->view;
    PropertiesDialog::connect_to_open_by_double_click(results_view, ADCONFIG()->get_column_index(ATTRIBUTE_DN));

    view_menu->addAction(find_widget->find_results->get_customize_columns_action());

    auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->setMenuBar(menubar);
    layout->addWidget(find_widget);

    connect(
        action_menu, &QMenu::aboutToShow,
        [=]() {
            find_widget->find_results->load_menu(action_menu);
        });
}
