/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

/**
 * @file scope_view.h
 * @brief Tree view for the console widget.
 */

#ifndef SCOPE_VIEW_H
#define SCOPE_VIEW_H

#include <QTreeView>

class ScopeView : public QTreeView {
public:
    using QTreeView::QTreeView;
    void resizeEvent(QResizeEvent *event) override;
};

#endif /* ifndef SCOPE_VIEW_H */
