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
    AttributeMultiEdit(QCheckBox *check, QList<AttributeMultiEdit *> &edits_out, QObject *parent);

    bool apply(AdInterface &ad, const QList<QString> &target_list);
    void reset();

private slots:
    void on_check_toggled();

signals:
    void edited();

protected:
    QCheckBox *apply_check;
    
    virtual bool apply_internal(AdInterface &ad, const QString &target) = 0;
    virtual void set_enabled(const bool enabled) = 0;
};

#define DECL_ATTRIBUTE_MULTI_EDIT_VIRTUALS()                              \
protected:                                                                \
    bool apply_internal(AdInterface &ad, const QString &target) override; \
    void set_enabled(const bool enabled) override;                        \
                                                                          \
public:

void multi_edits_connect_to_tab(const QList<AttributeMultiEdit *> &edits, PropertiesMultiTab *tab);

#endif /* ATTRIBUTE_MULTI_EDIT_H */
