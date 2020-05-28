
#include "actions.h"

QAction action_advanced_view("Advanced view");
QAction action_toggle_dn("Show DN");
QAction action_attributes("Attributes");
QAction action_delete_entry("Delete");
QAction action_new_user("New User");
QAction action_new_computer("New Computer");
QAction action_new_group("New Group");
QAction action_new_ou("New OU");

void actions_init() {
    action_advanced_view.setCheckable(true);
    action_toggle_dn.setCheckable(true);
}
