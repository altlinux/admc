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

#ifndef FILTER_WIDGET_ADVANCED_TAB_H
#define FILTER_WIDGET_ADVANCED_TAB_H

/**
 * Allows input of a plain LDAP filter string.
 */

#include "filter_widget/filter_widget.h"

class QPlainTextEdit;

class FilterWidgetAdvancedTab final : public FilterWidgetTab {
Q_OBJECT

public:
    FilterWidgetAdvancedTab();

    QString get_filter() const;

    void save_state(QHash<QString, QVariant> &state) const;
    void load_state(const QHash<QString, QVariant> &state);

private:
    QPlainTextEdit *ldap_filter_edit;
};

#endif /* FILTER_WIDGET_ADVANCED_TAB_H */
