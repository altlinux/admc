#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include "ad_defines.h"

#include <QObject>
#include <QIcon>
#include <QMap>

enum ItemIconType {
    ItemIconType_Policy_Clean,
    ItemIconType_OU_Clean,
    ItemIconType_OU_InheritanceBlocked,
    ItemIconType_Policy_Link,
    ItemIconType_Policy_Link_Disabled,
    ItemIconType_Policy_Enforced,
    ItemIconType_Policy_Enforced_Disabled,
    ItemIconType_Domain_Clean,
    ItemIconType_Domain_InheritanceBlocked,
    ItemIconType_Person_Clean,
    ItemIconType_Person_Blocked,
    ItemIconType_Site_Clean,
//    ItemIconType_Container_Clean,
//    ItemIconType_Configuration,
//    ItemIconType_Settings,

    ItemIconType_LAST
};

class AdObject;

class IconManager final {
public:
    explicit IconManager();

    void init();

    const QIcon& get_icon_for_type(ItemIconType icon_type) const;
    QIcon get_object_icon(const AdObject &object) const;
    QIcon get_object_icon(const QString& object_category) const;
    void set_icon_for_type(const QIcon &icon, ItemIconType icon_type);

private:
    QIcon type_index_icons_array[ItemIconType_LAST];
    QMap<QString, QList<QString>> category_to_icon_list;
    QString error_icon;

    //Enums positions where scope item icon can be overlayed
    //by another icon
    enum IconOverlayPosition {
        IconOverlayPosition_TopLeft,
        IconOverlayPosition_BottomLeft,
        IconOverlayPosition_TopRight,
        IconOverlayPosition_BottomRight
    };

    QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon,
                                         IconOverlayPosition position = IconOverlayPosition_BottomRight) const;
    QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon,
                                         const QSize &overlay_icon_size, const QPoint &pos) const;
};

#endif // ICON_MANAGER_H
