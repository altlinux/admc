#include "icon_manager.h"
#include "ad_defines.h"
#include "utils.h"
#include "ad_utils.h"
#include "ad_object.h"
#include "settings.h"
#include "globals.h"
#include "status.h"

#include <QPainter>
#include <QPixmap>
#include <QAction>
#include <QDir>
#include <QTextStream>
#include <QDebug>

IconManager::IconManager()
{
}

void IconManager::init(QMap<QString, QAction *> category_action_map)
{
    append_actions(category_action_map);

    // NOTE: use a list of possible icons because
    // default icon themes for different DE's don't
    // fully intersect
    category_to_icon_list = {
        {OBJECT_CATEGORY_DOMAIN_DNS, {"network-server"}},
        {OBJECT_CATEGORY_CONTAINER, {"folder"}},
        {OBJECT_CATEGORY_OU, {"folder-documents"}},
        {OBJECT_CATEGORY_GROUP, {"system-users"}},
        {OBJECT_CATEGORY_PERSON, {"avatar-default", "avatar-default-symbolic"}},
        {OBJECT_CATEGORY_COMPUTER, {"computer"}},
        {OBJECT_CATEGORY_GP_CONTAINER, {"preferences-other"}},
        {OBJECT_CATEGORY_VOLUME, {"folder-templates"}},
        {OBJECT_CATEGORY_SERVERS_CONTAINER, {"folder"}},
        {OBJECT_CATEGORY_SITE, {"go-home"}},

        // These categories are not AD object categories. They are used within ADMC context
        {ADMC_CATEGORY_QUERY_ITEM, {"document-send"}},
        {ADMC_CATEGORY_QUERY_FOLDER, {"folder"}},
        {ADMC_CATEGORY_ALL_POLICIES_FOLDER, {"folder"}},
        {ADMC_CATEGORY_GP_OBJECTS, {"folder"}},
        {ADMC_CATEGORY_FSMO_ROLE_CONTAINER, {"applications-system"}},
        {ADMC_CATEGORY_FSMO_ROLE, {"emblem-system"}},
        {ADMC_CATEGORY_DOMAIN_INFO_ITEM, {"network-workgroup"}},

        // Icons for some system containers and objects
        {OBJECT_CATEGORY_BUILTIN, {"emblem-system", "emblem-system-symbolic"}},
        {OBJECT_CATEGORY_LOST_AND_FOUND, {"emblem-system", "emblem-system-symbolic"}},
        {OBJECT_CATEGORY_INFRASTRUCTURE_UPDATE, {"emblem-system", "emblem-system-symbolic"}},
        {OBJECT_CATEGORY_MSDS_QUOTA_CONTAINER, {"emblem-system", "emblem-system-symbolic"}},
        {OBJECT_CATEGORY_PSO, {"preferences-desktop-personal"}},
        {OBJECT_CATEGORY_PSO_CONTAINER, {"preferences-desktop"}},
    };

    // Indicator icons
    indicator_map = {
        {inheritance_indicator, {"changes-prevent"}},
        {enforced_indicator, {"stop"}},
        {block_indicator, {"dialog-error"}},
        {link_indicator, {"mail-forward"}},
        {search_indicator, {"system-search"}},
        {warning_indicator, {"dialog-warning"}}
    };

    // NOTE: This is the icon used when no icon is
    // defined for some object category
    error_icon = "dialog-question";

    QString custom_themes_path = settings_get_variant(SETTING_custom_icon_themes_path).toString();
    if (custom_themes_path.isEmpty()) {
        custom_themes_path = "/usr/share/ad-integration-themes";
        settings_set_variant(SETTING_custom_icon_themes_path, custom_themes_path);
    }
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << custom_themes_path);

    system_theme = QIcon::themeName();

    const QString current_theme = settings_get_variant(SETTING_current_icon_theme).toString();
    const bool theme_is_available = get_available_themes().contains(current_theme);
    if (theme_is_available) {
        set_theme(current_theme);
    }
    else {
        set_theme(system_theme);
        if (!current_theme.isEmpty()) {
            g_status->add_message(QObject::tr("Theme from settings not found. System theme is set."), StatusType_Error);
        }
    }
}

QIcon IconManager::overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, IconOverlayPosition position) const
{
    QIcon overlapped_icon;
    //Icon looks not distorted with 16x16 size
    QPixmap original_pixmap = clean_icon.pixmap(32, 32);
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

QIcon IconManager::overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, const QSize &clean_icon_size, const QSize &overlay_icon_size, const QPoint &pos) const
{
    QIcon overlapped_icon;
    //Icon looks not distorted with 16x16 size
    QPixmap original_pixmap = clean_icon.pixmap(clean_icon_size.height(), clean_icon_size.width());
    QPixmap overlay_pixmap = overlay_icon.pixmap(overlay_icon_size);

    QPainter painter(&original_pixmap);
    painter.drawPixmap(pos.x(), pos.y(), overlay_pixmap);

    overlapped_icon.addPixmap(original_pixmap);
    return overlapped_icon;
}

void IconManager::update_action_icons() {
    for (const QString &category : category_action_map.keys()) {
        category_action_map[category]->setIcon(get_object_icon(category));
    }
}

void IconManager::update_icons_array() {
    type_index_icons_array[ItemIconType_Policy_Clean] = get_object_icon(OBJECT_CATEGORY_GP_CONTAINER);
    type_index_icons_array[ItemIconType_Policy_Link] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Policy_Clean], get_indicator_icon(link_indicator),
                                                                  QSize(16, 16), QSize(12, 12), QPoint(-2, 6));
    type_index_icons_array[ItemIconType_Policy_Link_Disabled] = type_index_icons_array[ItemIconType_Policy_Link].pixmap(16, 16, QIcon::Disabled);
    type_index_icons_array[ItemIconType_Policy_Enforced] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Policy_Link], get_indicator_icon(enforced_indicator),
                                                                                   QSize(16, 16), QSize(8, 8), QPoint(8, 8));
    type_index_icons_array[ItemIconType_Policy_Enforced_Disabled] = type_index_icons_array[ItemIconType_Policy_Enforced].pixmap(16, 16, QIcon::Disabled);
    type_index_icons_array[ItemIconType_OU_Clean] = get_object_icon(OBJECT_CATEGORY_OU);
    type_index_icons_array[ItemIconType_OU_InheritanceBlocked] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_OU_Clean], get_indicator_icon(inheritance_indicator),
                                                                            QSize(16, 16), QSize(10, 10), QPoint(6, 6));
    type_index_icons_array[ItemIconType_Domain_Clean] = get_object_icon(OBJECT_CATEGORY_DOMAIN_DNS);
    type_index_icons_array[ItemIconType_Domain_InheritanceBlocked] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Domain_Clean],
                                                                                get_indicator_icon(inheritance_indicator),
                                                                                QSize(16, 16), QSize(10, 10), QPoint(6, 6));
    type_index_icons_array[ItemIconType_Person_Clean] = get_object_icon(OBJECT_CATEGORY_PERSON).pixmap(max_icon_size);
    type_index_icons_array[ItemIconType_Person_Blocked] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Person_Clean],
                                                                                  get_indicator_icon(block_indicator), max_icon_size,
                                                                                  QSize(max_icon_size.width()/2, max_icon_size.height()/2),
                                                                                  QPoint(max_icon_size.width()/2, max_icon_size.width()/2));
    type_index_icons_array[ItemIconType_Site_Clean] = get_object_icon(OBJECT_CATEGORY_SITE);
    type_index_icons_array[ItemIconType_Computer_Clean] = get_object_icon(OBJECT_CATEGORY_COMPUTER).pixmap(max_icon_size);
    type_index_icons_array[ItemIconType_Computer_Blocked] = overlay_scope_item_icon(type_index_icons_array[ItemIconType_Computer_Clean],
                                                                                    get_indicator_icon(block_indicator), max_icon_size,
                                                                                    QSize(max_icon_size.width()/2, max_icon_size.height()/2),
                                                                                    QPoint(max_icon_size.width()/2, max_icon_size.width()/2));
    type_index_icons_array[ItemIconType_Group_Clean] = get_object_icon(OBJECT_CATEGORY_GROUP).pixmap(max_icon_size);
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
            fallback_icon_name,
            "emblem-system",
            "emblem-system-symbolic",
            "dialog-question",
        };

        QList<QString> icon_name_list = category_to_icon_list.value(object_category, fallback_icon_list);
        icon_name_list.prepend(object_category);

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

QIcon IconManager::get_indicator_icon(const QString &indicator_icon_name) const {
    if (indicator_icon_name.isEmpty()) {
        return QIcon::fromTheme(error_icon);
    }

    QList<QString> icon_name_list = indicator_map[indicator_icon_name];
    icon_name_list.prepend(indicator_icon_name);
    icon_name_list.append(fallback_icon_name);

    QIcon icon;
    for (const QString &icon_name : icon_name_list) {
        if (QIcon::hasThemeIcon(icon_name)) {
            icon = QIcon::fromTheme(icon_name);
            return icon;
        }
    }

    icon = QIcon::fromTheme(error_icon);
    return icon;
}

void IconManager::set_theme(const QString &icons_theme) {
    if (theme == icons_theme && !icons_theme.isEmpty()) {
        return;
    }

    theme = icons_theme.isEmpty() ? QIcon::fallbackThemeName() : icons_theme;

    QIcon::setThemeName(icons_theme);
    settings_set_variant(SETTING_current_icon_theme, icons_theme);
    update_action_icons();
    update_icons_array();
}

void IconManager::append_actions(const QMap<QString, QAction *> &categorized_actions) {
    category_action_map.insert(categorized_actions);
}

QStringList IconManager::get_available_themes() {
    QStringList available_themes = {system_theme};

    // Check existence and add themes from custom path
    const QDir custom_themes_dir = settings_get_variant(SETTING_custom_icon_themes_path).toString();

    for (const QString &theme_dir : custom_themes_dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QDir dir(custom_themes_dir.filePath(theme_dir));

        if (dir.exists("index.theme")) {
            available_themes.append(theme_dir);
        }
    }
    return available_themes;
}

QString IconManager::get_localized_theme_name(const QLocale locale, const QString &theme) {
    // Map is used for concatenation "Name" string with corresponding
    // language string/regexp in [] brackets for localized name search
    const QMap<QLocale::Language, QString> language_string_map = {
        {QLocale::Russian, "[ru]"}
    };
    const QLocale::Language language = locale.language();

    const QString search_string = "Name=";
    const QString search_string_localized = language_string_map.contains(language) ? QString("Name%1=").arg(language_string_map[language]):
                                                                                     QString();
    const bool theme_is_system = theme == system_theme;
    const QDir theme_dir = theme_is_system ? QDir(system_icons_dir_path).filePath(theme) :
                                                                 QDir(settings_get_variant(SETTING_custom_icon_themes_path).
                                                                      toString()).filePath(theme);
    QFile index_theme_file(theme_dir.filePath("index.theme"));
    index_theme_file.open(QIODevice::ReadOnly);

    QString theme_name;
    QTextStream in(&index_theme_file);

    QString line;
    do {
        line = in.readLine();

        if (line.contains(search_string, Qt::CaseSensitive)) {
            theme_name = line.remove(search_string);
        }

        if (!search_string_localized.isEmpty() &&
                line.contains(search_string_localized, Qt::CaseSensitive)) {
            theme_name = line.remove(search_string_localized);
            break;
        }
    } while (!line.isNull());

    index_theme_file.close();

    if (theme_is_system) {
        theme_name += QObject::tr(" (System)");
    }
    return theme_name;
}
