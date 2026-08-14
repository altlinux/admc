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

#ifndef SELECT_BASE_WIDGET_H
#define SELECT_BASE_WIDGET_H

/**
 * Allows user to select a search base object.
 */

#include <QWidget>

namespace Ui {
class SelectBaseWidget;
}

class SelectBaseWidget final : public QWidget {
    Q_OBJECT

public:
    Ui::SelectBaseWidget *ui;

    SelectBaseWidget(QWidget *parent = nullptr);
    ~SelectBaseWidget();

    void set_default_base(const QString &default_base);

    QString get_base() const;

    QVariant save_state() const;
    void restore_state(const QVariant &state);

private:
    void open_browse_dialog();
};

#endif /* SELECT_BASE_WIDGET_H */
