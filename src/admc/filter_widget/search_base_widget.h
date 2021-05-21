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

#ifndef SEARCH_BASE_WIDGET_H
#define SEARCH_BASE_WIDGET_H

/**
 * Allows user to select a search base object.
 */

#include <QWidget>

class QComboBox;
class QString;

class SearchBaseWidget final : public QWidget {
Q_OBJECT

public:
    SearchBaseWidget(const QString &default_search_base = QString());

    QString get_search_base() const;

    void serialize(QHash<QString, QVariant> &state) const;
    void deserialize(const QHash<QString, QVariant> &state);

private:
    QComboBox *combo;

    void browse();
};

#endif /* SEARCH_BASE_WIDGET_H */
