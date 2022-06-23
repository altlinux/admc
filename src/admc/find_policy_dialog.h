/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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

#ifndef FIND_POLICY_DIALOG_H
#define FIND_POLICY_DIALOG_H

/**
 * Find policy objects and perform actions on them.
 */

#include <QDialog>

namespace Ui {
class FindPolicyDialog;
}

class FindPolicyDialog final : public QDialog {
    Q_OBJECT

public:
    Ui::FindPolicyDialog *ui;

    FindPolicyDialog(QWidget *parent);
    ~FindPolicyDialog();

private:
        void add_filter();
};

#endif /* FIND_POLICY_DIALOG_H */