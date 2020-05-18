
#ifndef AD_INTERFACE_H
#define AD_INTERFACE_H

#include <QList>
#include <QString>
#include <QMap>

// Interface functions to convert from raw char** active directory returns to Qt containers
// Also can load fake data if program is run with "fake" option

enum NewEntryType {
    User,
    Computer,
    OU,
    Group
};

extern bool FAKE_AD; 

bool ad_interface_login();
QList<QString> load_children(const QString &dn);
QMap<QString, QList<QString>> load_attributes(const QString &dn);
bool set_attribute(const QString &dn, const QString &attribute, const QString &value);
bool create_entry(const QString &name, const QString &dn, const QString &parent_dn, NewEntryType type);

#endif /* AD_INTERFACE_H */
