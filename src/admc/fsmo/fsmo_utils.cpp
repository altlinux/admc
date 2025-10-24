#include "fsmo_utils.h"

#include "ad_config.h"
#include "globals.h"
#include "adldap.h"
#include "utils.h"
#include "settings.h"
#include "console_widget/console_widget.h"
#include "status.h"

#include <QString>
#include <QModelIndex>

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
    QString current_master = current_master_for_role(ad, role);
    settings_set_variant(SETTING_host, current_master);
    AdInterface::set_dc(current_master);
    ad.update_dc();
}

void connect_to_PDC_emulator(AdInterface &ad, ConsoleWidget *console)
{
    connect_host_with_role(ad, FSMORole_PDCEmulation);
    console->refresh_scope(console->domain_info_index());
    g_status->add_message(QObject::tr("PDC-Emulator is connected"), StatusType_Success);
}

QString current_master_for_role(AdInterface &ad, FSMORole role)
{
    QString role_dn = dn_from_role(role);
    return current_master_for_role_dn(ad, role_dn);
}

QString string_fsmo_role(FSMORole role)
{
    switch (role) {
        case FSMORole_DomainDNS: return "Domain DNS";
        case FSMORole_ForestDNS: return "Forest DNS";
        case FSMORole_PDCEmulation: return "PDC Emulator";
        case FSMORole_Schema: return "Schema master";
        case FSMORole_DomainNaming: return "Domain naming master";
        case FSMORole_Infrastructure: return "Infrastructure master";
        case FSMORole_RidAllocation: return "RID master";

        case FSMORole_COUNT: break;
    };

    return QString();
}

QString fsmo_string_from_dn(const QString &fsmo_role_dn)
{
    for (int role = 0; role < FSMORole_COUNT; ++role) {
        if (dn_from_role(FSMORole(role)) == fsmo_role_dn) {
            return string_fsmo_role(FSMORole(role));
        }
    }
    return QString();
}

bool fsmo_role_from_dn(const QString &role_dn, FSMORole &role_out) {
    const QString domain_dn = g_adconfig->domain_dn();
    const QString schema_dn = g_adconfig->schema_dn();
    const QString partitions_dn = g_adconfig->partitions_dn();

    const QString dn_lower = role_dn.toLower();

    if (dn_lower == domain_dn.toLower()) {
        role_out = FSMORole_PDCEmulation;
        return true;
    }

    if (dn_lower == schema_dn.toLower()) {
        role_out = FSMORole_Schema;
        return true;
    }

    if (dn_lower == partitions_dn.toLower()) {
        role_out = FSMORole_DomainNaming;
        return true;
    }

    if (dn_lower == QString("cn=infrastructure,dc=domaindnszones,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_DomainDNS;
        return true;
    }

    if (dn_lower == QString("cn=infrastructure,dc=forestdnszones,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_ForestDNS;
        return true;
    }

    if (dn_lower == QString("cn=infrastructure,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_Infrastructure;
        return true;
    }

    if (dn_lower == QString("cn=rid manager$,cn=system,%1").arg(domain_dn).toLower()) {
        role_out = FSMORole_RidAllocation;
        return true;
    }

     return false;
}
