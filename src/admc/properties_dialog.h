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

#ifndef PROPERTIES_DIALOG_H
#define PROPERTIES_DIALOG_H

/**
 * Shows info about object's attributes in multiple tabs.
 * Targeted at a particular object. Normally, a new dialog
 * is opened for each target. If a dialog is already opened
 * for selected target, it is focused.
 */

#include <QDialog>

class PropertiesTab;
class QAbstractItemView;
class QPushButton;
class AttributesTab;
class AdInterface;
class PropertiesWarningDialog;

namespace Ui {
class PropertiesDialog;
}

class PropertiesDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::PropertiesDialog *ui;

    // "dialog_is_new" flag is set to true if this is a
    // newly created dialog and false if a dialog was
    // already open for given target and reused. Use to know
    // whether to connect to applied() signal
    static PropertiesDialog *open_for_target(const QString &target, bool *dialog_is_new = nullptr);
    static void open_when_view_item_activated(QAbstractItemView *view, const int dn_role);

    ~PropertiesDialog();

signals:
    // Emitted when changes are applide via apply or ok
    // button
    void applied();

private slots:
    void accept() override;
    void apply();
    void reset();
    void on_edited();

private:
    QList<PropertiesTab *> tabs;
    QString target;
    QPushButton *apply_button;
    QPushButton *reset_button;
    AttributesTab *attributes_tab;
    PropertiesWarningDialog *warning_dialog;
    bool is_modified;

    // NOTE: ctor is private, use open_for_target() instead
    PropertiesDialog(const QString &target_arg);
    bool apply_internal(AdInterface &ad);
    void reset_internal(AdInterface &ad);

    void on_current_tab_changed(QWidget *prev_tab, QWidget *new_tab);
    void on_warning_dialog_accepted();
    void on_warning_dialog_rejected();
};

#endif /* PROPERTIES_DIALOG_H */
