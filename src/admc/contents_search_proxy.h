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

#ifndef CONTENTS_PROXY_H
#define CONTENTS_PROXY_H

#include <QSortFilterProxyModel>
#include <QString>

/**
 * Proxy model for containers widget that searches by name.
 * Plain QSortFilterProxyModel+regexp doesn't work for this purpose
 * due to messy root index stuff.
 */

class ContentsSearchProxy final : public QSortFilterProxyModel {
Q_OBJECT

public:
    using QSortFilterProxyModel::QSortFilterProxyModel;

    void update_root_index(const QModelIndex &root_index_arg);
    void update_search_text(const QString &search_text_arg);
    
private:
    QString search_text;
    QString root_dn;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* CONTENTS_PROXY_H */
