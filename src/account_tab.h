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

#ifndef ACCOUNT_TAB_H
#define ACCOUNT_TAB_H

#include "details_tab.h"
#include "ad_interface.h"

#include <QList>

class QString;
class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;
class AttributeEdit;

class AccountTab final : public DetailsTab {
Q_OBJECT

public:
    AccountTab(DetailsWidget *details_arg);

    void apply();
    bool accepts_target() const;

private slots:
    void on_unlock_button();
    void on_expiry_never_check();
    void on_expiry_set_check();
    void on_expiry_edit_button();
    
private:
    QList<AttributeEdit *> edits;

    // QCheckBox *expiry_never_check;
    // QCheckBox *expiry_set_check;
    // QLabel *expiry_display;
    // QPushButton *expiry_edit_button;

    void reload_internal();
};

#endif /* ACCOUNT_TAB_H */
