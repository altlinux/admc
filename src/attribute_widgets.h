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

#include "ad_interface.h"

#include <QString>
#include <QList>
#include <QMap>

class QLineEdit;
class QCheckBox;
class QGridLayout;
class QLabel;
class QWidget;

void layout_labeled_widget(QGridLayout *layout, QLabel *label, QWidget *widget);

void make_attribute_edits(const QList<QString> attributes, QGridLayout *layout, QMap<QString, QLineEdit *> *edits_out);
QList<AdResult> apply_attribute_edits(const QMap<QString, QLineEdit *> &edits, const QString &dn);
void show_warnings_for_error_results(const QList<AdResult> &results, QWidget *parent);
bool no_errors(const QList<AdResult> &results);

void make_account_option_checks(const QList<AccountOption> options, QGridLayout *layout, QMap<AccountOption, QCheckBox *> *checks_out);
QList<AdResult> apply_account_option_checks(const QMap<AccountOption, QCheckBox *> &checks, const QString &dn);
