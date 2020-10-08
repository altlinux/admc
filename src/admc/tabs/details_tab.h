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

#ifndef DETAILS_TAB_H
#define DETAILS_TAB_H

#include "ad_object.h"

#include <QWidget>
#include <QString>
#include <QList>

class AttributeEdit;

class DetailsTab : public QWidget {
Q_OBJECT

public:
    // Load original attribute values
    virtual void load(const AdObject &object);

    // Reset input to original attribute values
    virtual void reset();

    virtual bool changed() const;
    virtual bool verify() const;
    virtual void apply(const QString &target) const;

protected:
    QList<AttributeEdit *> edits;

signals:
    void edited();
    
public slots:
    void on_edit_edited();
};

#endif /* DETAILS_TAB_H */
