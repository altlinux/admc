#ifndef FSMO_UTILS_H
#define FSMO_UTILS_H

class QString;
class AdInterface;
class ConsoleWidget;

extern bool gpo_edit_without_PDC_disabled;

enum FSMORole {
    FSMORole_DomainDNS,
    FSMORole_ForestDNS,
    FSMORole_PDCEmulation,
    FSMORole_Schema,
    FSMORole_DomainNaming,
    FSMORole_Infrastructure,
    FSMORole_RidAllocation,

    FSMORole_COUNT,
};

QString string_fsmo_role(FSMORole role);

QString fsmo_string_from_dn(const QString &fsmo_role_dn);

// Returns the DN of the object that
// store's role's master in it's attributes
QString dn_from_role(FSMORole role);

FSMORole fsmo_role_from_dn(const QString &role_dn);

QString current_master_for_role(AdInterface &ad, FSMORole role);

// Returns dns host name of role's master object (DC)
QString current_master_for_role_dn(AdInterface &ad, QString role_dn);

bool current_dc_is_master_for_role(AdInterface &ad, FSMORole role);

void connect_host_with_role(AdInterface &ad, FSMORole role);

void connect_to_PDC_emulator(AdInterface &ad, ConsoleWidget *console);

#endif // FSMO_UTILS_H
