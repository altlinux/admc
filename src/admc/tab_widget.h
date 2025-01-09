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

#ifndef TAB_WIDGET_H
#define TAB_WIDGET_H

/**
 * Shows tabs in a horizontal list to the left and current
 * tab to the right. Used as a replacement for QTabWidget
 * because it doesn't have multi-line tabs.
 */

#include <QWidget>

namespace Ui {
class TabWidget;
}

class TabWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::TabWidget *ui;

    TabWidget(QWidget *parent = nullptr);
    ~TabWidget();

    QWidget *get_tab(const int index) const;
    QWidget *get_current_tab() const;
    void set_current_tab(const int index);
    void add_tab(QWidget *tab, const QString &title);

    // By default tab is automatically switches when
    // user selects different tab in tab list. If this
    // is disabled, then parent widget needs to connect
    // to current_changed() signal and manually change
    // tab using set_current_tab(). This is useful in
    // cases where you want to forbid switching tabs.
    void enable_auto_switch_tab(const bool enabled);

signals:
    void current_changed(const int prev, const int current);

private:
    void on_list_current_row_changed(int index);
    bool auto_switch_tab;
    bool ignore_current_row_signal;
};

#endif /* TAB_WIDGET_H */
