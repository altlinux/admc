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

#include <QList>
#include <QObject>

class QFormLayout;
class AdInterface;
class PropertiesMultiTab;
class QLabel;
class QCheckBox;

/**
 * Base class for attribute multi edits.
 */

class AttributeMultiEdit : public QObject {
    Q_OBJECT
public:
    AttributeMultiEdit(QList<AttributeMultiEdit *> &edits_out, QObject *parent);

    virtual void add_to_layout(QFormLayout *layout) = 0;
    bool apply(AdInterface &ad, const QList<QString> &target_list);
    void reset();

private slots:
    void on_check_toggled();

signals:
    void edited();

protected:
    QLabel *label;
    QWidget *check_and_label_wrapper;

    virtual bool apply_internal(AdInterface &ad, const QString &target) = 0;
    virtual void set_enabled(const bool enabled) = 0;

private:
    QCheckBox *apply_check;
};

#define DECL_ATTRIBUTE_MULTI_EDIT_VIRTUALS()                              \
public:                                                                   \
    void add_to_layout(QFormLayout *layout) override;                     \
                                                                          \
protected:                                                                \
    bool apply_internal(AdInterface &ad, const QString &target) override; \
    void set_enabled(const bool enabled) override;                        \
                                                                          \
public:

void multi_edits_connect_to_tab(const QList<AttributeMultiEdit *> &edits, PropertiesMultiTab *tab);
void multi_edits_add_to_layout(const QList<AttributeMultiEdit *> &edits, QFormLayout *layout);

#endif /* ATTRIBUTE_MULTI_EDIT_H */
