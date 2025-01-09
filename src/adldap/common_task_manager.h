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

#ifndef COMMONTASKMANAGER_H
#define COMMONTASKMANAGER_H

#include <QHash>

struct SecurityRight;
class AdConfig;

// NOTE: Each common task can represent combination of different
// rights, including common rights.
// Tasks are not equal extended rights with similar name.
enum CommonTask {
    CommonTask_UserControl, // User creation, deletion and control
    CommonTask_ChangeUserPassword, // Reset user password and force password change at next logon
    CommonTask_ReadAllUserInformation, // Read all user information
    CommonTask_GroupControl, // Group creation, deletion and control
    CommonTask_InetOrgPersonControl, // InetOrgPerson creation, deletion and control
    CommonTask_ChangeInetOrgPersonPassword, // Reset user password and force password change at next logon
    CommonTask_ReadAllInetOrgPersonInformation, // Read all inetOrgPerson information
    CommonTask_GroupMembership, // Modify the membership of a group
    CommonTask_ManageGPLinks, // Manage Group Policy links
    CommonTask_DomainComputerJoin, // Join a computer to the domain

    CommonTask_COUNT
};

/** Manages common tasks (the concept is taken from RSAT Users and Computers delegation).
 *  Theoretically, existing tasks can be supplemented with other tasks. This makes
 *  the manager class look more useful.
 */
class CommonTaskManager {
public:
    QHash<CommonTask, QList<SecurityRight>> common_task_rights;
    QHash<CommonTask, QString> common_task_name;
    QHash<QString, QList<CommonTask>> class_common_task_rights_map;

    explicit CommonTaskManager();
    void init(AdConfig *adconfig);

    QList<SecurityRight> rights_for_class(const QString &obj_class) const;
    QList<QString> tasks_object_classes() const;

private:
    AdConfig *ad_conf;
    void load_common_tasks_rights();
};

#endif // COMMONTASKMANAGER_H
