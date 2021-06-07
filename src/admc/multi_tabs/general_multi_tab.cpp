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

#include "multi_tabs/general_multi_tab.h"

#include "edits/string_edit.h"
#include "edits/string_other_edit.h"
#include "edits/country_edit.h"
#include "edits/group_scope_edit.h"
#include "edits/group_type_edit.h"
#include "adldap.h"
#include "multi_edits/string_multi_edit.h"

#include <QLabel>
#include <QFormLayout>
#include <QFrame>

GeneralMultiTab::GeneralMultiTab(const QList<QString> &class_list) {   
    auto name_label = new QLabel(tr("Multiple objects selected"));

    auto explanation_label = new QLabel(tr("To change a property for multiple objects, first select the checkbox to enable the change and then type the change. Depending on the number of selected objects, you might have to wait while the changes are applied."));
    explanation_label->setWordWrap(true);

    auto line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    auto edit_layout = new QFormLayout();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(name_label);
    top_layout->addWidget(explanation_label);
    top_layout->addWidget(line);
    top_layout->addLayout(edit_layout);

    if (class_list == QList<QString>({CLASS_USER})) {
        new StringMultiEdit(ATTRIBUTE_DESCRIPTION, edit_list, edit_layout);
        new StringMultiEdit(ATTRIBUTE_OFFICE, edit_list, edit_layout);
        new StringMultiEdit(ATTRIBUTE_MOBILE, edit_list, edit_layout);
        new StringMultiEdit(ATTRIBUTE_FAX_NUMBER, edit_list, edit_layout);
        new StringMultiEdit(ATTRIBUTE_WWW_HOMEPAGE, edit_list, edit_layout);
        new StringMultiEdit(ATTRIBUTE_MAIL, edit_list, edit_layout);
    } else {
        new StringMultiEdit(ATTRIBUTE_DESCRIPTION, edit_list, edit_layout);
    }

    multi_edits_add_to_layout(edit_list, edit_layout);
    multi_edits_connect_to_tab(edit_list, this);
}
