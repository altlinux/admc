/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
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
 * Dialog wrapper over filter widget. Contains the filter
 * widget and ok/cancel buttons.
 */

#include <QDialog>
#include <QVariant>

namespace Ui {
class FilterDialog;
}

class FilterDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::FilterDialog *ui;

    FilterDialog(const QList<QString> &class_list, const QList<QString> &selected_list, QWidget *parent);
    ~FilterDialog();

    QVariant save_state() const;
    void restore_state(const QVariant &state);
    void enable_filtering_all_classes();

    QString get_filter() const;

private:
    QVariant original_state;
};

#endif /* FILTER_DIALOG_H */
