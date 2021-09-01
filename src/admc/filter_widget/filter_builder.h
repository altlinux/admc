/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

#ifndef FILTER_BUILDER_H
#define FILTER_BUILDER_H

/**
 * Build a single LDAP filter step by step by selecting
 * attribute, condition and value. Attributes are split into
 * groups by class to which they are related.
 */

#include <QWidget>

class QComboBox;
class QLineEdit;
class AdConfig;

class FilterBuilder final : public QWidget {
    Q_OBJECT

public:
    FilterBuilder(AdConfig *adconfig);

    QString get_filter() const;
    QString get_filter_display() const;
    void clear();

private slots:
    void update_attributes_combo();
    void update_conditions_combo();
    void update_value_edit();

private:
    AdConfig *adconfig;
    QComboBox *attribute_class_combo;
    QComboBox *attribute_combo;
    QComboBox *condition_combo;
    QLineEdit *value_edit;
};

#endif /* FILTER_BUILDER_H */
