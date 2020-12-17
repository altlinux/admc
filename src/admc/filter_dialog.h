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

#ifndef FILTER_DIALOG_H
#define FILTER_DIALOG_H

/**
 * Contains FilterWidget. When a filter is entered and
 * dialog is accepted, emits filter_changed() signal. Used
 * for filtering ObjectModel.
 */

#include <QDialog>

class FilterDialog final : public QDialog {
Q_OBJECT

public:
    FilterDialog(QWidget *parent);

signals:
    void filter_changed(const QString &filter);
};

#endif /* FILTER_DIALOG_H */
