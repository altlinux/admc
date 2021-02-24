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

#ifndef PROPERTIES_TAB_H
#define PROPERTIES_TAB_H

#include <QWidget>
#include <QString>
#include <QList>

/**
 * PropertiesTab are used in PropertiesDialog. They are usually
 * a collection of AttributeEdit's and so mirror their
 * behavior. All tab f-ns call f-ns of tab's edits but edits
 * aren't required. If needed, a tab can implement custom
 * functionality without edits.
 */

class AttributeEdit;
class AdInterface;
class AdObject;

class PropertiesTab : public QWidget {
Q_OBJECT

public:
    virtual void load(AdInterface &ad, const AdObject &object);
    virtual bool verify(AdInterface &ad, const QString &target) const;
    virtual void apply(AdInterface &ad, const QString &target);

protected:
    QList<AttributeEdit *> edits;

signals:
    // Emitted to notify details dialog that tab was edited.
    void edited();
    
public slots:
    void on_edit_edited();
};

#endif /* PROPERTIES_TAB_H */
