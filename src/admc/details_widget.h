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

#ifndef DETAILS_WIDGET_H
#define DETAILS_WIDGET_H

#include <QDialog>
#include <QString>

class QString;
class QTabWidget;
class QLabel;
class DetailsTab;
class QDialogButtonBox;

enum TabHandle {
    TabHandle_General,
    TabHandle_Object,
    TabHandle_Attributes,
    TabHandle_Account,
    TabHandle_Members,
    TabHandle_Address,
    TabHandle_COUNT
};

// Shows info about object's attributes in multiple tabs
// Targeted at a particular object
// Updates targets of all tabs when target changes
class DetailsWidget final : public QDialog {
Q_OBJECT

public:
    DetailsWidget(const DetailsWidget&) = delete;
    DetailsWidget& operator=(const DetailsWidget&) = delete;
    DetailsWidget(DetailsWidget&&) = delete;
    DetailsWidget& operator=(DetailsWidget&&) = delete;

    static DetailsWidget *docked_instance();
    static void change_target(const QString &new_target);

    QString get_target() const;
    void tab_edited(DetailsTab *tab);
    void reload(const QString &new_target);

private slots:
    void on_logged_in();
    void on_ad_modified();
    void on_apply();
    void on_cancel();
    void on_docked_setting_changed();

private:
    bool is_floating_instance;
    QTabWidget *tab_widget = nullptr;
    QLabel *title_label = nullptr;
    QDialogButtonBox *button_box = nullptr;
    DetailsTab *tabs[TabHandle_COUNT];
    QString target;

    DetailsWidget(bool is_floating_instance_arg);
};

#endif /* DETAILS_WIDGET_H */
