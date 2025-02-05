/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#ifndef LAPS_V2_TAB_H
#define LAPS_V2_TAB_H

#include <QWidget>

class AttributeEdit;

namespace Ui {
class LAPSV2Tab;
}

class LAPSV2Tab final : public QWidget {
    Q_OBJECT

public:
    Ui::LAPSV2Tab *ui;

    LAPSV2Tab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~LAPSV2Tab();
private slots:
    void on_show_password_button_toggled(bool checked);
    void on_copy_password_button_clicked();
    void on_expiration_datetimeedit_dateTimeChanged(const QDateTime &dateTime);
};

#endif /* LAPS_V2_TAB_H */
