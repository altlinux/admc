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

#ifndef MOVE_OBJECT_DIALOG_H
#define MOVE_OBJECT_DIALOG_H

/**
 *
 */

#include "select_container_dialog.h"

class MoveObjectDialog final : public SelectContainerDialog {
Q_OBJECT

public:
    MoveObjectDialog(const QList<QString> &targets_arg, QWidget *parent);

    QList<QString> get_moved_objects() const;

public slots:
    void accept() override;

private:
    QList<QString> targets;
    QList<QString> moved_objects;
};

#endif /* MOVE_OBJECT_DIALOG_H */
