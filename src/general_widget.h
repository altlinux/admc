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

#ifndef GENERAL_WIDGET_H
#define GENERAL_WIDGET_H

#include "ad_interface.h"

#include <QWidget>
#include <QList>
#include <QString>

class QString;
class QLineEdit;
class QCheckBox;
class QLabel;
class QPushButton;

// Shows member objects of targeted group
class GeneralWidget final : public QWidget {
Q_OBJECT

public:
    GeneralWidget(QWidget *parent);

    void change_target(const QString &dn);

private slots:

signals:
    void target_changed();

private:
    QString target_dn;
    QLabel *name_label;
};

#endif /* GENERAL_WIDGET_H */
