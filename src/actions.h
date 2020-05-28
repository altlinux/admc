
#ifndef ACTIONS_H
#define ACTIONS_H

#include <QAction>

extern QAction action_advanced_view;
extern QAction action_toggle_dn;
extern QAction action_attributes;
extern QAction action_delete_entry;
extern QAction action_new_user;
extern QAction action_new_computer;
extern QAction action_new_group;
extern QAction action_new_ou;

void actions_init();

#endif /* ACTIONS_H */
