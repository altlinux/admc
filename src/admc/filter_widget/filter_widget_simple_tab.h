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

#ifndef FILTER_WIDGET_SIMPLE_TAB_H
#define FILTER_WIDGET_SIMPLE_TAB_H

/**
 * Simple filter with just a class selection and name input.
 */

#include "filter_widget/filter_widget.h"

class QLineEdit;
class SelectClassesWidget;

class FilterWidgetSimpleTab final : public FilterWidgetTab {
Q_OBJECT

public:
    FilterWidgetSimpleTab();

    QString get_filter() const;

signals:
    void returnPressed();

private:
    SelectClassesWidget *select_classes;
    QLineEdit *name_edit;
};

#endif /* FILTER_WIDGET_SIMPLE_TAB_H */
