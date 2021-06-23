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

#ifndef PROPERTIES_MULTI_TAB_H
#define PROPERTIES_MULTI_TAB_H

#include <QWidget>

class AdInterface;
class AttributeMultiEdit;

/**
 * Base class for properties multi tabs.
 */

class PropertiesMultiTab : public QWidget {
    Q_OBJECT

public:
    virtual bool apply(AdInterface &ad, const QList<QString> &target_list);
    virtual void reset();

signals:
    void edited();

public slots:
    void on_edit_edited();

protected:
    QList<AttributeMultiEdit *> edit_list;
};

#endif /* PROPERTIES_MULTI_TAB_H */
