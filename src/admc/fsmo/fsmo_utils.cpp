#include "fsmo_utils.h"

#include "ad_config.h"
#include "globals.h"
#include "adldap.h"
#include "utils.h"
#include "settings.h"

#include <QString>

bool gpo_edit_without_PDC_disabled = true;

QString dn_from_role(FSMORole role)
{
    const QString domain_dn = g_adconfig->domain_dn();

    switch (role) {
        case FSMORole_DomainDNS: return QString("CN=Infrastructure,DC=DomainDnsZones,%1").arg(domain_dn);
        case FSMORole_ForestDNS: return QString("CN=Infrastructure,DC=ForestDnsZones,%1").arg(domain_dn);
        case FSMORole_PDCEmulation: return domain_dn;
        case FSMORole_Schema: return g_adconfig->schema_dn();
        case FSMORole_DomainNaming: return g_adconfig->partitions_dn();
        case FSMORole_Infrastructure: return QString("CN=Infrastructure,%1").arg(domain_dn);
        case FSMORole_RidAllocation: return QString("CN=RID Manager$,CN=System,%1").arg(domain_dn);

        case FSMORole_COUNT: break;
    };

    return QString();
}

QString current_master_for_role_dn(AdInterface &ad, QString role_dn)
{
    const AdObject role_object = ad.search_object(role_dn);
    const QString master_settings_dn = role_object.get_string(ATTRIBUTE_FSMO_ROLE_OWNER);
    const QString master_dn = dn_get_parent(master_settings_dn);
    const AdObject master_object = ad.search_object(master_dn);
    const QString current_master = master_object.get_string(ATTRIBUTE_DNS_HOST_NAME);
    return current_master;
}

bool current_dc_is_master_for_role(AdInterface &ad, FSMORole role)
{
    QString role_dn = dn_from_role(role);
    QString current_master = current_master_for_role_dn(ad, role_dn);
    QString current_dc = current_dc_dns_host_name(ad);
    return current_master == current_dc;
}

void connect_host_with_role(AdInterface &ad, FSMORole role)
{
    QString role_dn = dn_from_role(role);
    QString current_master = current_master_for_role_dn(ad, role_dn);
    settings_set_variant(SETTING_host, current_master);
    AdInterface::set_dc(current_master);
    ad.update_dc();
}
