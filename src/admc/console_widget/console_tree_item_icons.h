#ifndef CONSOLETREEITEMICONS_H
#define CONSOLETREEITEMICONS_H

#include <QIcon>

enum ItemIconType {
    ItemIconType_Policy_Clean,
    ItemIconType_OU_Clean,
    ItemIconType_OU_InheritanceBlocked,
    ItemIconType_Policy_Link,
    ItemIconType_Policy_Enforced,
    ItemIconType_Domain_Clean,
    ItemIconType_Domain_InheritanceBlocked,

    ItemIconType_LAST
};

const QIcon& get_console_tree_item_icon(ItemIconType icon_type);

#endif // CONSOLETREEITEMICONS_H
