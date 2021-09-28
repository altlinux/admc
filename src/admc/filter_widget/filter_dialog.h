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

#ifndef FILTER_DIALOG_H
#define FILTER_DIALOG_H

/**
 * Dialog wrapper over filter widget. Contains the filter
 * widget and ok/cancel buttons.
 */

#include <QDialog>
#include <QVariant>

class FilterWidget;
class AdConfig;

namespace Ui {
    class FilterDialog;
}

class FilterDialog final : public QDialog {
    Q_OBJECT

public:
    FilterDialog(QWidget *parent);

    void open() override;
    void reject() override;

    void add_classes(AdConfig *adconfig, const QList<QString> &class_list);

    QVariant save_state() const;
    void restore_state(const QVariant &state);

    QString get_filter() const;

private:
    Ui::FilterDialog *ui;
    FilterWidget *filter_widget;
    QVariant original_state;
};

#endif /* FILTER_DIALOG_H */
