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

#ifndef ATTRIBUTE_MULTI_EDIT_H
#define ATTRIBUTE_MULTI_EDIT_H

#include <QObject>

class AdInterface;
class PropertiesMultiTab;
class QCheckBox;

/**
 * Base class for attribute multi edits.
 */

class AttributeMultiEdit : public QObject {
    Q_OBJECT
public:
    AttributeMultiEdit(QCheckBox *check, QList<AttributeMultiEdit *> *edit_list, QObject *parent);

    bool need_to_apply() const;
    void uncheck();

    virtual bool apply(AdInterface &ad, const QString &target) = 0;
    virtual void set_enabled(const bool enabled) = 0;

signals:
    void edited();

private slots:
    void on_apply_check();

private:
    QCheckBox *apply_check;
};

#endif /* ATTRIBUTE_MULTI_EDIT_H */
