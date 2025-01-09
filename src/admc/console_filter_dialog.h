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

#ifndef CONSOLE_FILTER_DIALOG_H
#define CONSOLE_FILTER_DIALOG_H

/**
 * Dialog used to enter a filter that is applied to objects
 * in the console. Allows showing all objects, objects of
 * certain type or entering a custom filter.
 */

#include <QDialog>
#include <QVariant>

class QRadioButton;

namespace Ui {
class ConsoleFilterDialog;
};

class ConsoleFilterDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::ConsoleFilterDialog *ui;

    ConsoleFilterDialog(QWidget *parent);
    ~ConsoleFilterDialog();

    void accept() override;

    QVariant save_state() const;
    void restore_state(const QVariant &state);

    QString get_filter() const;
    bool get_filter_enabled() const;

private:
    QHash<QString, QRadioButton *> button_state_name_map;
    QVariant original_state;
    QVariant filter_dialog_state;
    QString custom_filter;

    void open_custom_dialog();
    void on_custom_button();
    void on_classes_button();
};

#endif /* CONSOLE_FILTER_DIALOG_H */
