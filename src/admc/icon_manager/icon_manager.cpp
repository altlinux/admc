#include "icon_manager.h"
#include "ad_defines.h"
#include "utils.h"
#include "ad_utils.h"
#include "ad_object.h"

#include <QPainter>
#include <QPixmap>

IconManager::IconManager()
{
}

void IconManager::init()
{
    // NOTE: use a list of possible icons because
    // default icon themes for different DE's don't
    // fully intersect
    category_to_icon_list = {
        {"Domain-DNS", {"network-server"}},
        {"Container", {"folder"}},
        {OBJECT_CATEGORY_OU, {"folder-documents"}},
        {OBJECT_CATEGORY_GROUP, {"system-users"}},
        {OBJECT_CATEGORY_PERSON, {"avatar-default", "avatar-default-symbolic"}},
        {"Computer", {"computer"}},
        {"Group-Policy-Container", {"preferences-other"}},
        {"Volume", {"folder-templates"}},

        // Some custom icons for one-off objects
        {"Builtin-Domain", {"emblem-system", "emblem-system-symbolic"}},
        {"Configuration", {"emblem-system", "emblem-system-symbolic"}},
        {"Lost-And-Found", {"emblem-system", "emblem-system-symbolic"}},
        {"Infrastructure-Update", {"emblem-system", "emblem-system-symbolic"}},
        {"ms-DS-Quota-Container", {"emblem-system", "emblem-system-symbolic"}},
    };

    // NOTE: This is the icon used when no icon is
    // defined for some object category
    error_icon = "dialog-question";

    type_index_icons_array[ItemIconType_Policy_Clean] = get_object_icon("Group-Policy-Container");
    type_index_icons_array[ItemIconType_Policy_Link] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Policy_Clean], QIcon::fromTheme("mail-forward"),
                                                                  QSize(12, 12), QPoint(-2, 6));
    type_index_icons_array[ItemIconType_Policy_Link_Disabled] = type_index_icons_array[ItemIconType_Policy_Link].pixmap(16, 16, QIcon::Disabled);
    type_index_icons_array[ItemIconType_Policy_Enforced] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Policy_Link], QIcon::fromTheme("stop"));
    type_index_icons_array[ItemIconType_Policy_Enforced_Disabled] = type_index_icons_array[ItemIconType_Policy_Enforced].pixmap(16, 16, QIcon::Disabled);
    type_index_icons_array[ItemIconType_OU_Clean] = get_object_icon(OBJECT_CATEGORY_OU);
    type_index_icons_array[ItemIconType_OU_InheritanceBlocked] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_OU_Clean], QIcon::fromTheme("changes-prevent"),
                                                                            QSize(10, 10), QPoint(6, 6));
    type_index_icons_array[ItemIconType_Domain_Clean] = get_object_icon("Domain-DNS");
    type_index_icons_array[ItemIconType_Domain_InheritanceBlocked] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Domain_Clean],
                                                                                QIcon::fromTheme("changes-prevent"),
                                                                                QSize(10, 10), QPoint(6, 6));
    type_index_icons_array[ItemIconType_Person_Clean] = get_object_icon(OBJECT_CATEGORY_PERSON);
    type_index_icons_array[ItemIconType_Person_Blocked] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Person_Clean],
                                                                                  QIcon::fromTheme("dialog-error"), QSize(8, 8), QPoint(8, 8));
    type_index_icons_array[ItemIconType_Site_Clean] = QIcon::fromTheme("go-home");
}

QIcon IconManager::overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, IconOverlayPosition position) const
{
    QIcon overlapped_icon;
    //Icon looks not distorted with 16x16 size
    QPixmap original_pixmap = clean_icon.pixmap(16, 16);
    QPixmap overlay_pixmap = overlay_icon.pixmap(10, 10);

    QPainter painter(&original_pixmap);
    switch (position)
    {
    case IconOverlayPosition_BottomLeft:
        painter.drawPixmap(0, original_pixmap.height() - 8, overlay_pixmap);
        break;
    case IconOverlayPosition_TopLeft:
        painter.drawPixmap(0, 0, overlay_pixmap);
        break;
    case IconOverlayPosition_TopRight:
        painter.drawPixmap(original_pixmap.width() - 8, 0, overlay_pixmap);
        break;
    case IconOverlayPosition_BottomRight:
        painter.drawPixmap(original_pixmap.width() - 8, original_pixmap.height() - 8, overlay_pixmap);
        break;
    default:
        break;
    }

    overlapped_icon.addPixmap(original_pixmap);

    return overlapped_icon;
}

QIcon IconManager::overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, const QSize &overlay_icon_size, const QPoint &pos) const
{
    QIcon overlapped_icon;
    //Icon looks not distorted with 16x16 size
    QPixmap original_pixmap = clean_icon.pixmap(16, 16);
    QPixmap overlay_pixmap = overlay_icon.pixmap(overlay_icon_size);

    QPainter painter(&original_pixmap);
    painter.drawPixmap(pos.x(), pos.y(), overlay_pixmap);

    overlapped_icon.addPixmap(original_pixmap);
    return overlapped_icon;
}

const QIcon &IconManager::get_icon_for_type(ItemIconType icon_type) const
{
    const QIcon &icon = type_index_icons_array[icon_type];
    return icon;
}

QIcon IconManager::get_object_icon(const AdObject &object) const
{
    const QString object_category = [&]() {
        const QString category_dn = object.get_string(ATTRIBUTE_OBJECT_CATEGORY);
        const QString out = dn_get_name(category_dn);

        return out;
    }();

    const QIcon out = get_object_icon(object_category);

    return out;
}

QIcon IconManager::get_object_icon(const QString &object_category) const
{
    const QString icon_name = [&]() -> QString {
        const QList<QString> fallback_icon_list = {
            "emblem-system",
            "emblem-system-symbolic",
            "dialog-question",
        };
        const QList<QString> icon_name_list = category_to_icon_list.value(object_category, fallback_icon_list);

        for (const QString &icon : icon_name_list) {
            if (QIcon::hasThemeIcon(icon)) {
                return icon;
            }
        }

        return error_icon;
    }();

    const QIcon icon = QIcon::fromTheme(icon_name);
    return icon;
}

void IconManager::set_icon_for_type(const QIcon &icon, ItemIconType icon_type)
{
    type_index_icons_array[icon_type] = icon;
}
