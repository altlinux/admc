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

#include <QWidget>
#include <QString>

class DetailsWidget;
class QLayout;
class QLineEdit;

enum OldAttributeEditType {
    OldAttributeEditType_ReadOnly,
    OldAttributeEditType_Editable
};

class DetailsTab : public QWidget {
Q_OBJECT

public:
    DetailsTab(DetailsWidget *details_arg);
    
    QString target() const;
    QString get_title() const;

    void reload();
    virtual void apply() = 0;
    virtual bool accepts_target() const = 0;

    DetailsWidget *details;

protected:
    QString title;

    // Implement this in subclass to reload member widgets

    void add_attribute_edit(const QString &attribute, const QString &label_text, QLayout *label_layout, QLayout *edit_layout, OldAttributeEditType type);

private:
    struct OldAttributeEdit {
        QString attribute;
        QLineEdit *edit;
    };

    QList<OldAttributeEdit> attribute_edits;

    virtual void reload_internal() = 0;
    void reload_attribute_edit(QLineEdit *edit, const QString &attribute);
};

#endif /* DETAILS_TAB_H */
