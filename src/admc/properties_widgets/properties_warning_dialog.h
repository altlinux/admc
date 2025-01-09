/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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

#ifndef PROPERTIES_WARNING_DIALOG_H
#define PROPERTIES_WARNING_DIALOG_H

/**
 * Dialog which opens when switching to/from attributes tab
 * while there are un-applied changes. Let's user choose
 * whether to apply changes and move to new tab or stay on
 * current tab.
 */

#include <QDialog>

enum PropertiesWarningType {
    PropertiesWarningType_SwitchToAttributes,
    PropertiesWarningType_SwitchFromAttributes,
};

namespace Ui {
class PropertiesWarningDialog;
}

class PropertiesWarningDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::PropertiesWarningDialog *ui;

    PropertiesWarningDialog(const PropertiesWarningType type, QWidget *parent);
    ~PropertiesWarningDialog();

signals:
    void applied();
    void discarded();

private:
    void on_apply_button();
    void on_discard_button();
};

#endif /* PROPERTIES_WARNING_DIALOG_H */
