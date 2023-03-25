#include "console_tree_item_icons.h"
#include "ad_defines.h"
#include "utils.h"

#include <QPainter>
#include <QPixmap>

//Enums positions where scope item icon can be overlayed
//by another icon
enum IconOverlayPosition {
    IconOverlayPosition_TopLeft,
    IconOverlayPosition_BottomLeft,
    IconOverlayPosition_TopRight,
    IconOverlayPosition_BottomRight
};

/**
 * ConsoleTreeItemIcons struct contains array with icons that
 * are set to appropriate item when group policy object changes its state
 */
struct ConsoleTreeItemIcons final
{
    QIcon icons_map[ItemIconType_LAST];

    ConsoleTreeItemIcons();
};

static QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon,
                                     IconOverlayPosition position = IconOverlayPosition_BottomRight);
static QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon,
                                     const QSize &overlay_icon_size, const QPoint &pos);

ConsoleTreeItemIcons::ConsoleTreeItemIcons()
{
    icons_map[ItemIconType_Policy_Clean] = get_object_icon("Group-Policy-Container");
    icons_map[ItemIconType_Policy_Link] = overlay_scope_item_icon(icons_map[ItemIconType_Policy_Clean], QIcon::fromTheme("mail-forward"),
                                                                  QSize(12, 12), QPoint(-2, 6));
    icons_map[ItemIconType_Policy_Link_Disabled] = icons_map[ItemIconType_Policy_Link].pixmap(16, 16, QIcon::Disabled);
    icons_map[ItemIconType_Policy_Enforced] = overlay_scope_item_icon(icons_map[ItemIconType_Policy_Link], QIcon::fromTheme("stop"));
    icons_map[ItemIconType_Policy_Enforced_Disabled] = icons_map[ItemIconType_Policy_Enforced].pixmap(16, 16, QIcon::Disabled);
    icons_map[ItemIconType_OU_Clean] = get_object_icon(OBJECT_CATEGORY_OU);
    icons_map[ItemIconType_OU_InheritanceBlocked] = overlay_scope_item_icon(icons_map[ItemIconType_OU_Clean], QIcon::fromTheme("changes-prevent"),
                                                                            QSize(10, 10), QPoint(6, 6));
    icons_map[ItemIconType_Domain_Clean] = get_object_icon("Domain-DNS");
    icons_map[ItemIconType_Domain_InheritanceBlocked] = overlay_scope_item_icon(icons_map[ItemIconType_Domain_Clean],
                                                                                QIcon::fromTheme("changes-prevent"),
                                                                                QSize(10, 10), QPoint(6, 6));
}

static QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, IconOverlayPosition position)
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

static QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, const QSize &overlay_icon_size, const QPoint &pos)
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

const QIcon &get_console_tree_item_icon(ItemIconType icon_type)
{
    static const ConsoleTreeItemIcons item_icons;

    const QIcon &icon = item_icons.icons_map[icon_type];
    return icon;
}
