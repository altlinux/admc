
#ifndef CREATE_ENTRY_H
#define CREATE_ENTRY_H

#include "ad_interface.h"

void create_entry_dialog(NewEntryType type, const QString &given_parent_dn = "");
void create_user_dialog();
void create_computer_dialog();
void create_ou_dialog();
void create_group_dialog();

#endif /* CREATE_ENTRY_H */
