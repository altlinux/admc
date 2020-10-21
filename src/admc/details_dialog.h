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

#ifndef DETAILS_DIALOG_H
#define DETAILS_DIALOG_H

#include <QDialog>
#include <QString>
#include <QList>

class QString;
class QTabWidget;
class QLabel;
class DetailsTab;
class QDialogButtonBox;

/**
 * Shows info about object's attributes in multiple tabs.
 * Targeted at a particular object. Normally, a new dialog
 * is opened for each target. So can have multiple dialogs
 * open a the same time. If docked setting is turned on,
 * only one dialog is shown, docked to the right side of
 * main window. When a new target is selected, that one
 * docked dialog switches to it.
 */

class DetailsDialog final : public QDialog {
Q_OBJECT

public:
    static QWidget *get_docked_container();

    static void open_for_target(const QString &target);

    QString get_target() const;

private slots:
    void on_apply();
    void on_docked_setting_changed();
    void on_tab_edited();
    void on_ad_modified();

private:
    static DetailsDialog *docked_instance;
    bool is_floating_instance;
    QTabWidget *tab_widget = nullptr;
    QLabel *title_label = nullptr;
    QDialogButtonBox *button_box = nullptr;
    QList<DetailsTab *> tabs;
    QString target;

    DetailsDialog(const QString &target_arg, bool is_floating_instance_arg);
    void load();
    
    DetailsDialog(const DetailsDialog&) = delete;
    DetailsDialog& operator=(const DetailsDialog&) = delete;
    DetailsDialog(DetailsDialog&&) = delete;
    DetailsDialog& operator=(DetailsDialog&&) = delete;
};

#endif /* DETAILS_DIALOG_H */
