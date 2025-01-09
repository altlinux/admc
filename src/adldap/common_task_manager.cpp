/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "common_task_manager.h"

#include "ad_security.h"
#include "ad_config.h"
#include "ad_defines.h"

#include <QCoreApplication>

void CommonTaskManager::init(AdConfig *adconfig) {
    ad_conf = adconfig;

    class_common_task_rights_map[CLASS_USER] = {CommonTask_UserControl,
                                           CommonTask_ChangeUserPassword,
                                           CommonTask_ReadAllUserInformation};
    class_common_task_rights_map[CLASS_GROUP] = {CommonTask_GroupControl,
                                                 CommonTask_GroupMembership};
    class_common_task_rights_map[CLASS_INET_ORG_PERSON] = {CommonTask_InetOrgPersonControl,
                                                      CommonTask_ChangeInetOrgPersonPassword,
                                                      CommonTask_ReadAllInetOrgPersonInformation};
    class_common_task_rights_map[CLASS_OU] = {CommonTask_ManageGPLinks};
    class_common_task_rights_map[CLASS_DOMAIN] = {CommonTask_DomainComputerJoin};

    common_task_name[CommonTask_UserControl] = QCoreApplication::translate("CommonTaskManager", "Create, delete and manage user accounts");
    common_task_name[CommonTask_ChangeUserPassword] = QCoreApplication::translate("CommonTaskManager", "Reset user passwords and force password change at next logon");
    common_task_name[CommonTask_ReadAllUserInformation] = QCoreApplication::translate("CommonTaskManager", "Read all user information");
    common_task_name[CommonTask_GroupControl] = QCoreApplication::translate("CommonTaskManager", "Create, delete and manage groups");
    common_task_name[CommonTask_GroupMembership] = QCoreApplication::translate("CommonTaskManager", "Modify the membership of group");
    common_task_name[CommonTask_ManageGPLinks] = QCoreApplication::translate("CommonTaskManager", "Manage Group Policy links");
    common_task_name[CommonTask_InetOrgPersonControl] = QCoreApplication::translate("CommonTaskManager", "Create, delete and manage inetOrgPerson accounts");
    common_task_name[CommonTask_ChangeInetOrgPersonPassword] = QCoreApplication::translate("CommonTaskManager", "Reset inetOrgPerson passwords and force password change at next logon");
    common_task_name[CommonTask_ReadAllInetOrgPersonInformation] = QCoreApplication::translate("CommonTaskManager", "Read all inetOrgPerson information");
    common_task_name[CommonTask_DomainComputerJoin] = QCoreApplication::translate("CommonTaskManager", "Join a computer to the domain");

    load_common_tasks_rights();
}

QList<SecurityRight> CommonTaskManager::rights_for_class(const QString &obj_class) const {
    QList<SecurityRight> rights;
    if (!class_common_task_rights_map.keys().contains(obj_class)) {
        return rights;
    }

    for (CommonTask task : class_common_task_rights_map.value(obj_class)) {
        rights.append(common_task_rights[task]);
    }

    return rights;
}

QList<QString> CommonTaskManager::tasks_object_classes() const {
    return class_common_task_rights_map.keys();
}

CommonTaskManager::CommonTaskManager() {
}

void CommonTaskManager::load_common_tasks_rights() {
    if (!ad_conf) {
        return;
    }

    // Append creation, deletion and control rights for corresponding classes
    const QHash<QString, CommonTask> creation_deletion_control_map = {
        {CLASS_USER, CommonTask_UserControl},
        {CLASS_GROUP, CommonTask_GroupControl},
        {CLASS_INET_ORG_PERSON, CommonTask_InetOrgPersonControl},
    };

    for (const QString &obj_class : creation_deletion_control_map.keys()) {
        CommonTask permission = creation_deletion_control_map[obj_class];
        common_task_rights[permission] = creation_deletion_rights_for_class(ad_conf, obj_class) +
            control_children_class_right(ad_conf, obj_class);
    }

    // Append "Reset password and force password change at next logon"
    // rights for user and inetOrgPerson
    const QHash<QString, CommonTask> password_change_map = {
        {CLASS_USER, CommonTask_ChangeUserPassword},
        {CLASS_INET_ORG_PERSON, CommonTask_ChangeInetOrgPersonPassword},
    };

    for (const QString &obj_class : password_change_map.keys()) {
        CommonTask permission = password_change_map[obj_class];
        common_task_rights[permission] = children_class_read_write_prop_rights(ad_conf, obj_class, ATTRIBUTE_PWD_LAST_SET);
    }

    // Append "Read all information" rights for user and inetOrgPerson
    const QHash<QString, CommonTask> read_all_info_map = {
        {CLASS_USER, CommonTask_ReadAllUserInformation},
        {CLASS_INET_ORG_PERSON, CommonTask_ReadAllInetOrgPersonInformation},
    };

    for (const QString &obj_class : read_all_info_map.keys()) {
        CommonTask permission = read_all_info_map[obj_class];
        common_task_rights[permission] = read_all_children_class_info_rights(ad_conf, obj_class);
    }

    // Append group membership rights.
    // NOTE: Group membership task doesnt represent "Membership" extended right.
    // These generate different ACEs: Membership task delegation doesnt include
    // rights on memberOf attribute, that included in extended right's property set.
    // See https://learn.microsoft.com/en-us/windows/win32/adschema/r-membership.
    common_task_rights[CommonTask_GroupMembership] = children_class_read_write_prop_rights(ad_conf, CLASS_GROUP, ATTRIBUTE_MEMBER);

    // Append group policy link manage rights
    common_task_rights[CommonTask_ManageGPLinks] = read_write_property_rights(ad_conf, ATTRIBUTE_GPOPTIONS) +
            read_write_property_rights(ad_conf, ATTRIBUTE_GPLINK);

    // Append domain computer join rights
    common_task_rights[CommonTask_DomainComputerJoin] = create_children_class_right(ad_conf, CLASS_COMPUTER);
}
