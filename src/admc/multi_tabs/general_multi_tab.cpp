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

#include "multi_tabs/general_multi_tab.h"
#include "edits/string_edit.h"
#include "edits/string_other_edit.h"
#include "edits/country_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "adldap.h"

#include <QLabel>
#include <QFormLayout>
#include <QFrame>

GeneralMultiTab::GeneralMultiTab() {   
    auto name_label = new QLabel(tr("Multiple objects selected"));

    auto line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(name_label);
    top_layout->addWidget(line);
}
