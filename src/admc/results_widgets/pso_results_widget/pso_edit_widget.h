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

#ifndef PSO_EDIT_WIDGET_H
#define PSO_EDIT_WIDGET_H

#include <QWidget>

namespace Ui {
class PSOEditWidget;
}

class AdObject;
class QListWidgetItem;
class QLineEdit;
class QSpinBox;
class QCheckBox;

class PSOEditWidget final : public QWidget {
    Q_OBJECT

public:
    explicit PSOEditWidget(QWidget *parent = nullptr);
    ~PSOEditWidget();

    void update(const AdObject &passwd_settings_obj);
    void update_defaults();
    void set_read_only(bool read_only);

    /*!
     * Gets password setting attribute's values hash
     */
    QHash<QString, QList<QByteArray>> pso_settings_values();
    QHash<QString, QList<QString>> pso_settings_string_values();
    QStringList applied_dn_list() const;
    QLineEdit *name_line_edit();

    /*!
     * Compares current settings with default (except name and precedence).
     * Returns true if at least one is not default.
     */
    bool settings_are_default();

private:
    Ui::PSOEditWidget *ui;
    QStringList dn_applied_list;
    QHash<QString, QList<QByteArray>> default_setting_values;

    void on_add();
    void on_remove();

    /*!
     * Returns appropriate timespan unit value depending on given attribute.
     * It is used to fill password timespan setting checkboxes.
     */
    int spinbox_timespan_units(const AdObject &obj, const QString &attribute);

    enum AppliedItemRole {
        AppliedItemRole_DN = Qt::UserRole
    };
};

#endif // PSO_EDIT_WIDGET_H
