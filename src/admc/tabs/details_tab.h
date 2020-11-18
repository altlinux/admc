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

/**
 * Details tabs are used in DetailsDialog. They usually are
 * a collection of AttributeEdit's and so mirror their
 * behavior. Tabs start out unmodified, then enter
 * modified() when user edits them. Then tabs may be reset
 * via load() or verified and applied. All tab f-ns call
 * f-ns of tab's edits. Note that edits are not required. If
 * needed, a tab can have no edits at all and implement
 * custom functionality.
 */

class AttributeEdit;

class DetailsTab : public QWidget {
Q_OBJECT

public:
    virtual void load(const AdObject &object);
    virtual bool modified() const;
    virtual bool verify() const;
    virtual void apply(const QString &target) const;

protected:
    QList<AttributeEdit *> edits;

signals:
    // Emitted to notify details dialog that tab was edited.
    void edited();
    
public slots:
    void on_edit_edited();
};

#endif /* DETAILS_TAB_H */
