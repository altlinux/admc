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

#ifndef OBJECT_MULTI_PROPERTIES_DIALOG_H
#define OBJECT_MULTI_PROPERTIES_DIALOG_H

/**
 * TODO: comment
 */

#include <QDialog>

class PropertiesMultiTab;

class ObjectMultiPropertiesDialog final : public QDialog {
Q_OBJECT

public:
    ObjectMultiPropertiesDialog(const QList<QString> &target_list_arg, const QList<QString> &class_list);

signals:
    void applied();
    
private slots:
    void ok();
    void reset();
    void on_tab_edited();

private:
    QList<QString> target_list;
    QList<PropertiesMultiTab *> tab_list;
    QPushButton *apply_button;
    QPushButton *reset_button;

    bool apply();
};

#endif /* OBJECT_MULTI_PROPERTIES_DIALOG_H */
