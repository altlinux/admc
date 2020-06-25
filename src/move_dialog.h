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

#ifndef MOVE_DIALOG_H
#define MOVE_DIALOG_H

#include <QDialog>
#include <QString>
#include <QList>
#include <QStandardItemModel>

class QWidget;
class QLineEdit;
class QSortFilterProxyModel;
class QComboBox;
class MoveDialogModel;
class QTreeView;
class QLabel;
class QAction;

enum ClassFilter {
    ClassFilter_All,
    ClassFilter_Containers,
    ClassFilter_OUs,
    ClassFilter_COUNT
};
Q_DECLARE_METATYPE(ClassFilter)

class MoveDialog final : public QDialog {
Q_OBJECT

public:
    MoveDialog(QWidget *parent);

    void open_for_entry(const QString &dn);

private slots:
    void on_filter_name_changed(const QString &text);
    void on_filter_class_changed(int index);
    void on_double_clicked(const QModelIndex &index);

private:
    QTreeView *view = nullptr;
    QLabel *target_label = nullptr;
    QComboBox *filter_class_combo_box = nullptr;
    QLineEdit *filter_name_line_edit = nullptr;
    MoveDialogModel *model = nullptr;
    QSortFilterProxyModel *proxy_name = nullptr;
    QSortFilterProxyModel *proxy_class = nullptr;
    QString target_dn = "";
};

class MoveDialogModel final : public QStandardItemModel {
Q_OBJECT

public:
    MoveDialogModel(QObject *parent);

    void load(const QString &dn, QList<ClassFilter> classes);
};

#endif /* MOVE_DIALOG_H */
