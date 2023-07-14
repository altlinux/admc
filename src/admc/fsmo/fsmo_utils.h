#ifndef FSMO_UTILS_H
#define FSMO_UTILS_H

class QString;
class AdInterface;

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

// Returns the DN of the object that
// store's role's master in it's attributes
QString dn_from_role(FSMORole role);

// Returns dns host name of role's master object (DC)
QString current_master_for_role_dn(AdInterface &ad, QString role_dn);

bool current_dc_is_master_for_role(AdInterface &ad, FSMORole role);

void connect_host_with_role(AdInterface &ad, FSMORole role);

#endif // FSMO_UTILS_H
