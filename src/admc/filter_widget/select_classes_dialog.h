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

#ifndef SELECT_CLASSES_DIALOG_H
#define SELECT_CLASSES_DIALOG_H

/**
 * Dialog to select classes, opened from
 * SelectClassesWidget. Wraps FilterClassesWidget inside.
 */

#include <QDialog>
#include <QVariant>

class FilterClassesWidget;

namespace Ui {
    class SelectClassesDialog;
}

class SelectClassesDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::SelectClassesDialog *ui;

    SelectClassesDialog(QWidget *parent);
    ~SelectClassesDialog();

    void open() override;
    void reject() override;
    FilterClassesWidget *filter_classes_widget() const;

private:
    QVariant state_to_restore;

    void reset();
};

#endif /* SELECT_CLASSES_DIALOG_H */
