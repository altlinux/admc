#ifndef SITESLINKEDIT_H
#define SITESLINKEDIT_H

#include "attribute_edit.h"
#include "tabs/sites_link_tab/sites_link_type.h"

class SitesLinkWidget;
class SitesLinkCommonWidget;
class SitesLinkPartWidget;
class AdObject;
class AdInterface;
class StringEdit;

class SitesLinkEdit final : public AttributeEdit {
    Q_OBJECT

public:
    explicit SitesLinkEdit(SitesLinkWidget *link_wget_arg, QObject *parent);
    explicit SitesLinkEdit(SitesLinkType type_arg, SitesLinkCommonWidget *common_link_wget_arg, QObject *parent);

    void load(AdInterface &ad, const AdObject &object) override;
    bool apply(AdInterface &ad, const QString &dn) const override;
    bool verify(AdInterface &ad, const QString &dn) const;

private:
    // TODO: Add current edit current values (cost, description, etc)
    // for comparison on changes to avoid excessive queries
    SitesLinkCommonWidget *sites_link_common_wget;
    SitesLinkPartWidget *sites_link_part_wget;
    SitesLinkType type;
    bool is_link_type;
    StringEdit *description_edit;

    void setup_widgets();
};

#endif // SITESLINKEDIT_H
