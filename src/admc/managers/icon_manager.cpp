#include "icon_manager.h"
#include "ad_defines.h"
#include "utils.h"
#include "ad_utils.h"
#include "ad_object.h"
#include "settings.h"
#include "globals.h"
#include "status.h"

#include <QPainter>
#include <QIcon>
#include <QPixmap>
#include <QAction>
#include <QDir>
#include <QTextStream>
#include <QLocale>
#include <QDebug>

class IconManager::IconManagerImpl final {
public:
    IconManager *q;
    const QString search_indicator = "search-indicator";
    const QString warning_indicator = "warning-indicator";
    const QString link_indicator = "link-indicator";
    const QString block_indicator = "block-indicator";
    const QString enforced_indicator = "enforced-indicator";
    const QString inheritance_indicator = "inheritance-indicator";

    const QSize max_icon_size = QSize(64, 64);

    QIcon item_icons_array[ItemIcon_COUNT];
    QMap<QString, QList<QString>> category_icon_names_map;
    QMap<QString, QAction*> category_action_map;

    const QString error_icon_name = "error-icon";
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

    explicit IconManagerImpl(IconManager *parent);
    ~IconManagerImpl() = default;

    QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon,
                                         IconOverlayPosition position = IconOverlayPosition_BottomRight) const;
    QIcon overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, const QSize &clean_icon_size,
                                         const QSize &overlay_icon_size, const QPoint &pos) const;
    void update_icons_array();
    void update_action_icons();
    // Adds actions and their categories for further update.
    // NOTE: Categories may not correspond objectCategory object attribute,
    //       for example, query folder and query items are not AD objects
    void append_actions(const QMap<QString, QAction*> &categorized_actions);
    bool main_icons_are_valid();
};

IconManager::IconManagerImpl::IconManagerImpl(IconManager *parent) : q(parent) {
    // NOTE: use a list of possible icons because
    // default icon themes for different DE's don't
    // fully intersect
    category_icon_names_map = {
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
        {ADMC_CATEGORY_GO_PREVIOUS_ACTION, {"go-previous", "go-previous-symbolic"}},
        {ADMC_CATEGORY_GO_NEXT_ACTION, {"go-next", "go-next-symbolic"}},
        {ADMC_CATEGORY_GO_UP_ACTION, {"go-up", "go-up-symbolic"}},
        {ADMC_CATEGORY_REFRESH_ACTION, {"view-refresh", "view-refresh-symbolic"}},
        {ADMC_CATEGORY_MANUAL_ACTION, {"help"}},

        // Icons for some system containers and objects
        {OBJECT_CATEGORY_BUILTIN, {"emblem-system", "emblem-system-symbolic"}},
        {OBJECT_CATEGORY_LOST_AND_FOUND, {"emblem-system", "emblem-system-symbolic"}},
        {OBJECT_CATEGORY_MSDS_QUOTA_CONTAINER, {"Container", "emblem-system", "emblem-system-symbolic"}},
        {OBJECT_CATEGORY_PSO, {"preferences-desktop-personal"}},
        {OBJECT_CATEGORY_PSO_CONTAINER, {"preferences-desktop"}},

        // Indicator icons (aren't AD object categories too)
        {inheritance_indicator, {"changes-prevent"}},
        {enforced_indicator, {"stop"}},
        {block_indicator, {"dialog-error"}},
        {link_indicator, {"mail-forward"}},
        {search_indicator, {"system-search"}},
        {warning_indicator, {"dialog-warning"}},

        // NOTE: Error icon used when no icon is
        // defined for given object category
        {error_icon_name, {"dialog-question", "dialog-question-symbolic"}}
    };
}

QIcon IconManager::IconManagerImpl::overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, IconOverlayPosition position) const {
    QIcon overlapped_icon;

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

QIcon IconManager::IconManagerImpl::overlay_scope_item_icon(const QIcon &clean_icon, const QIcon &overlay_icon, const QSize &clean_icon_size,
                                                            const QSize &overlay_icon_size, const QPoint &pos) const {
    QIcon overlapped_icon;

    QPixmap original_pixmap = clean_icon.pixmap(clean_icon_size.height(), clean_icon_size.width());
    QPixmap overlay_pixmap = overlay_icon.pixmap(overlay_icon_size);

    QPainter painter(&original_pixmap);
    painter.drawPixmap(pos.x(), pos.y(), overlay_pixmap);

    overlapped_icon.addPixmap(original_pixmap);
    return overlapped_icon;
}

void IconManager::IconManagerImpl::update_action_icons() {
    for (const QString &category : category_action_map.keys()) {
        category_action_map[category]->setIcon(q->category_icon(category));
    }
}

void IconManager::IconManagerImpl::update_icons_array() {
    item_icons_array[ItemIcon_Block_Indicator] = q->category_icon(block_indicator);
    item_icons_array[ItemIcon_Warning_Indicator] = q->category_icon(warning_indicator);
    item_icons_array[ItemIcon_Inheritance_Block_Indicator] = q->category_icon(inheritance_indicator);
    item_icons_array[ItemIcon_Policy_Enforce_Indicator] = q->category_icon(enforced_indicator);
    item_icons_array[ItemIcon_Policy_Link_Indicator] = q->category_icon(link_indicator);

    item_icons_array[ItemIcon_Policy] = q->category_icon(OBJECT_CATEGORY_GP_CONTAINER);
    item_icons_array[ItemIcon_Policy_Link] = overlay_scope_item_icon(item_icons_array[ItemIcon_Policy], q->item_icon(ItemIcon_Policy_Link_Indicator),
                                                                  QSize(16, 16), QSize(12, 12), QPoint(-2, 7));
    item_icons_array[ItemIcon_Policy_Link_Disabled] = item_icons_array[ItemIcon_Policy_Link].pixmap(16, 16, QIcon::Disabled);
    item_icons_array[ItemIcon_Policy_Enforced] = overlay_scope_item_icon(item_icons_array[ItemIcon_Policy_Link], q->item_icon(ItemIcon_Policy_Enforce_Indicator),
                                                                                   QSize(16, 16), QSize(8, 8), QPoint(8, 8));
    item_icons_array[ItemIcon_Policy_Enforced_Disabled] = item_icons_array[ItemIcon_Policy_Enforced].pixmap(16, 16, QIcon::Disabled);
    item_icons_array[ItemIcon_OU] = q->category_icon(OBJECT_CATEGORY_OU);
    item_icons_array[ItemIcon_OU_InheritanceBlocked] = overlay_scope_item_icon(item_icons_array[ItemIcon_OU], q->item_icon(ItemIcon_Inheritance_Block_Indicator),
                                                                            QSize(16, 16), QSize(10, 10), QPoint(6, 6));
    item_icons_array[ItemIcon_Domain] = q->category_icon(OBJECT_CATEGORY_DOMAIN_DNS);
    item_icons_array[ItemIcon_Domain_InheritanceBlocked] = overlay_scope_item_icon(item_icons_array[ItemIcon_Domain],
                                                                                q->item_icon(ItemIcon_Inheritance_Block_Indicator),
                                                                                QSize(16, 16), QSize(10, 10), QPoint(6, 6));
    item_icons_array[ItemIcon_Person] = q->category_icon(OBJECT_CATEGORY_PERSON).pixmap(max_icon_size);
    item_icons_array[ItemIcon_Person_Blocked] = overlay_scope_item_icon(item_icons_array[ItemIcon_Person],
                                                                                  q->item_icon(ItemIcon_Block_Indicator), max_icon_size,
                                                                                  QSize(max_icon_size.width()/2, max_icon_size.height()/2),
                                                                                  QPoint(max_icon_size.width()/2, max_icon_size.width()/2));
    item_icons_array[ItemIcon_Site] = q->category_icon(OBJECT_CATEGORY_SITE);
    item_icons_array[ItemIcon_Computer] = q->category_icon(OBJECT_CATEGORY_COMPUTER).pixmap(max_icon_size);
    item_icons_array[ItemIcon_Computer_Blocked] = overlay_scope_item_icon(item_icons_array[ItemIcon_Computer],
                                                                                    q->item_icon(ItemIcon_Computer_Blocked), max_icon_size,
                                                                                    QSize(max_icon_size.width()/2, max_icon_size.height()/2),
                                                                                    QPoint(max_icon_size.width()/2, max_icon_size.width()/2));
    item_icons_array[ItemIcon_Group] = q->category_icon(OBJECT_CATEGORY_GROUP).pixmap(max_icon_size);
    item_icons_array[ItemIcon_Password_Settings_Object] = q->category_icon(OBJECT_CATEGORY_PSO);
    item_icons_array[ItemIcon_Password_Settings_Object] = q->category_icon(OBJECT_CATEGORY_PSO_CONTAINER);
}

const QIcon IconManager::item_icon(ItemIcon icon_type) const {
    const QIcon icon = impl->item_icons_array[icon_type];
    return icon;
}

QIcon IconManager::object_icon(const AdObject &object) const {
    const QString object_category = [&]() {
        const QString category_dn = object.get_string(ATTRIBUTE_OBJECT_CATEGORY);
        const QString out = dn_get_name(category_dn);

        return out;
    }();

    const QIcon out = category_icon(object_category);

    return out;
}

QIcon IconManager::category_icon(const QString &object_category) const {
    const QString icon_name = [&]() -> QString {
        const QList<QString> fallback_icon_list = {
            impl->fallback_icon_name,
            "emblem-system",
            "emblem-system-symbolic",
            "dialog-question",
        };

        QList<QString> icon_name_list = impl->category_icon_names_map.value(object_category, fallback_icon_list);
        icon_name_list.prepend(object_category);

        for (const QString &icon_name : icon_name_list) {
            if (QIcon::hasThemeIcon(icon_name)) {
                return icon_name;
            }
        }

        return impl->error_icon_name;
    }();

    const QIcon icon = QIcon::fromTheme(icon_name);
    return icon;
}

void IconManager::set_theme(const QString &icons_theme) {
    if (impl->theme == icons_theme && !icons_theme.isEmpty()) {
        return;
    }

    impl->theme = icons_theme.isEmpty() ? QIcon::fallbackThemeName() : icons_theme;

    QIcon::setThemeName(icons_theme);
    settings_set_variant(SETTING_current_icon_theme, icons_theme);
    impl->update_action_icons();
    impl->update_icons_array();
}

void IconManager::IconManagerImpl::append_actions(const QMap<QString, QAction *> &categorized_actions) {
    category_action_map.insert(categorized_actions);
}

QStringList IconManager::available_themes() {
    QStringList available_themes = {impl->system_theme};

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

QString IconManager::localized_theme_name(const QLocale locale, const QString &theme) {
    // Map is used for concatenation "Name" string with corresponding
    // language string/regexp in [] brackets for localized name search
    const QMap<QLocale::Language, QString> language_string_map = {
        {QLocale::Russian, "[ru]"}
    };
    const QLocale::Language language = locale.language();

    const QString search_string = "Name=";
    const QString search_string_localized = language_string_map.contains(language) ? QString("Name%1=").arg(language_string_map[language]):
                                                                                     QString();
    const bool theme_is_system = theme == impl->system_theme;
    const QDir theme_dir = theme_is_system ? QDir(impl->system_icons_dir_path).filePath(theme) :
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

IconManager::IconManager() : impl(std::unique_ptr<IconManagerImpl>(new IconManagerImpl(this))) {
}

void IconManager::init(const QMap<QString, QAction *> &category_action_map) {
    impl->append_actions(category_action_map);

    QString custom_themes_path = settings_get_variant(SETTING_custom_icon_themes_path).toString();
    if (custom_themes_path.isEmpty()) {
        custom_themes_path = "/usr/share/ad-integration-themes";
        settings_set_variant(SETTING_custom_icon_themes_path, custom_themes_path);
    }
    QIcon::setThemeSearchPaths(QIcon::themeSearchPaths() << custom_themes_path);

    impl->system_theme = QIcon::themeName();

    const QString current_theme = settings_get_variant(SETTING_current_icon_theme).toString();
    const bool theme_is_available = available_themes().contains(current_theme);
    if (theme_is_available) {
        set_theme(current_theme);
    }
    else {
        set_theme(impl->system_theme);
        if (!current_theme.isEmpty()) {
            g_status->add_message(QObject::tr("Theme from settings not found. System theme is set."), StatusType_Error);
        }
    }
}
