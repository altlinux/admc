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

#ifndef ACCOUNT_WIDGET_H
#define ACCOUNT_WIDGET_H

#include <QWidget>

class QString;
class QLineEdit;
class QLabel;
class QPushButton;

// Shows member objects of targeted group
class AccountWidget final : public QWidget {
Q_OBJECT

public:
    AccountWidget(QWidget *parent);

    void change_target(const QString &dn);

private slots:
    void on_lock_button();

private:
    QString target_dn;
    QLineEdit *logon_name_edit;
    QLabel *lock_label;
    QPushButton *lock_button;
};

#endif /* ACCOUNT_WIDGET_H */
