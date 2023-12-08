#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include "ad_defines.h"

#include <QObject>
#include <QIcon>
#include <QMap>
#include <QSize>
#include <QLocale>


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
    ItemIconType_Computer_Clean,
    ItemIconType_Computer_Blocked,
    ItemIconType_Group_Clean,

    ItemIconType_LAST
};

class AdObject;
class QAction;
template <typename T, typename U>
class QMap;
class ConsoleWidget;

class IconManager final {
public:
    const QString search_indicator = "search-indicator";
    const QString warning_indicator = "warning-indicator";
    const QString link_indicator = "link-indicator";
    const QString block_indicator = "block-indicator";
    const QString enforced_indicator = "enforced-indicator";
    const QString inheritance_indicator = "inheritance-indicator";

    const QSize max_icon_size = QSize(64, 64);

    explicit IconManager();

    void init(QMap<QString, QAction*> category_action_map);

    const QIcon& get_icon_for_type(ItemIconType icon_type) const;
    QIcon get_object_icon(const AdObject &object) const;
    QIcon get_object_icon(const QString& object_category) const;
    QIcon get_indicator_icon(const QString &indicator_icon_name) const;
    void set_theme(const QString &icons_theme);

    // Adds actions and their categories for further update.
    // NOTE: Categories may not correspond objectCategory object attribute,
    //       for example, query folder and query items are not AD objects
    void append_actions(const QMap<QString, QAction*> &categorized_actions);

    QStringList get_available_themes();
    QString get_localized_theme_name(const QLocale locale, const QString &theme);

private:
    QIcon type_index_icons_array[ItemIconType_LAST];
    QMap<QString, QList<QString>> category_to_icon_list;
    QMap<QString, QList<QString>> indicator_map;
    QMap<QString, QAction*> category_action_map;

    QString error_icon;
    const QString fallback_icon_name = "fallback";

    QString theme;
    // system_theme field contains current system theme
    // name that may not be fallback.
    QString system_theme;

    const QString system_icons_dir_path = "/usr/share/icons";

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
    QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, const QSize &clean_icon_size,
                                         const QSize &overlay_icon_size, const QPoint &pos) const;
    void update_icons_array();
    void update_action_icons();
};

#endif // ICON_MANAGER_H
