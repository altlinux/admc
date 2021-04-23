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

#include "object_multi_properties_dialog.h"

#include "tabs/properties_tab.h"
#include "tabs/attributes_tab.h"
#include "tabs/membership_tab.h"
#include "tabs/account_tab.h"
#include "tabs/general_tab.h"
#include "tabs/address_tab.h"
#include "tabs/object_tab.h"
#include "tabs/group_policy_tab.h"
#include "tabs/gpo_links_tab.h"
#include "tabs/organization_tab.h"
#include "tabs/telephones_tab.h"
#include "tabs/profile_tab.h"
#include "tabs/managed_by_tab.h"
#include "adldap.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "tab_widget.h"

#include <QAction>
#include <QLabel>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDebug>
#include <QAbstractItemView>

ObjectMultiPropertiesDialog::ObjectMultiPropertiesDialog(const QList<QString> &target_list_arg)
: QDialog()
{
    target_list = target_list_arg;

    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Properties for multiple objects"));
}
