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

#ifndef TELEPHONES_TAB_H
#define TELEPHONES_TAB_H

#include <QWidget>

class AttributeEdit;

namespace Ui {
class TelephonesTab;
}

class TelephonesTab final : public QWidget {
    Q_OBJECT

public:
    Ui::TelephonesTab *ui;

    TelephonesTab(QList<AttributeEdit *> *edit_list, QWidget *parent);
    ~TelephonesTab();
};

#endif /* TELEPHONES_TAB_H */
