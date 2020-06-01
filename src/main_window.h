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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "ad_interface.h"

#include <QMainWindow>

class QString;
class AdModel;
class ContainersWidget;
class ContentsWidget;
class AttributesWidget;

class MainWindow final : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow();

private slots:
    void on_action_attributes();
    void on_action_delete_entry();
    void on_action_new_user();
    void on_action_new_computer();
    void on_action_new_group();
    void on_action_new_ou();
    void on_containers_clicked_dn(const QString &dn);
    void on_contents_clicked_dn(const QString &dn);

private:
    QString get_selected_dn() const;
    void on_action_new_entry_generic(NewEntryType type);

    AdModel *ad_model;
    ContainersWidget *containers_widget;
    ContentsWidget *contents_widget;
    AttributesWidget *attributes_widget;

    QAction *action_containers_click_attributes = nullptr;
    QAction *action_contents_click_attributes = nullptr;
};

#endif /* MAIN_WINDOW_H */
