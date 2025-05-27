#ifndef ICON_MANAGER_H
#define ICON_MANAGER_H

#include <memory>

// Can be supplemented if needed
enum ItemIcon {
    ItemIcon_Policy,
    ItemIcon_OU,
    ItemIcon_OU_InheritanceBlocked,
    ItemIcon_Policy_Link,
    ItemIcon_Policy_Link_Disabled,
    ItemIcon_Policy_Enforced,
    ItemIcon_Policy_Enforced_Disabled,
    ItemIcon_Domain,
    ItemIcon_Domain_InheritanceBlocked,
    ItemIcon_Person,
    ItemIcon_Person_Blocked,
    ItemIcon_Site,
    ItemIcon_Computer,
    ItemIcon_Computer_Blocked,
    ItemIcon_Group,
    ItemIcon_Search_Indicator,
    ItemIcon_Warning_Indicator,
    ItemIcon_Policy_Link_Indicator,
    ItemIcon_Block_Indicator,
    ItemIcon_Policy_Enforce_Indicator,
    ItemIcon_Inheritance_Block_Indicator,

    ItemIcon_COUNT
};

class AdObject;
class QAction;
template <typename T, typename U>
class QMap;
class QLocale;
class QIcon;
class QString;
class QStringList;

class IconManager final {
public:
    explicit IconManager();

    void init(const QMap<QString, QAction*> &category_action_map);

    const QIcon item_icon(ItemIcon icon_type) const;
    QIcon object_icon(const AdObject &object) const;
    QIcon category_icon(const QString& object_category) const;
    void set_theme(const QString &icons_theme);

    QStringList available_themes();
    QString localized_theme_name(const QLocale locale, const QString &theme);

private:
    class IconManagerImpl;
    std::unique_ptr<IconManagerImpl> impl;
};

#endif // ICON_MANAGER_H
