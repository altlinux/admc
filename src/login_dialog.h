/**
 * Copyright (c) by: Mike Dawson mike _at_ no spam gp2x.org
 *
 * This file may be used subject to the terms and conditions of the
 * GNU Library General Public License Version 2, or any later version
 * at your option, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
**/

#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QStandardItemModel>

class QWidget;
class QLineEdit;
class QSortFilterProxyModel;
class QComboBox;
class QListWidget;
class QLabel;
class QAction;
class QListWidgetItem;
class QCheckBox;

class LoginDialog final : public QDialog {
Q_OBJECT

public:
    LoginDialog(QAction *login_action, QWidget *parent);

private slots:
    void on_host_double_clicked(QListWidgetItem *item);
    void on_login_button(bool);
    void on_cancel_button(bool);
    void on_finished();
    void show();

private:
    QListWidget *hosts_list = nullptr;
    QLineEdit *domain_edit = nullptr;
    QLineEdit *site_edit = nullptr;
    QCheckBox *save_session_checkbox = nullptr;

    void complete(const QString &host);
    void load_hosts();
};

#endif /* LOGIN_DIALOG_H */
