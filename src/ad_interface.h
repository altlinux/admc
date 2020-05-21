
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
    Group,
    COUNT
};

const QMap<NewEntryType, QString> new_entry_type_to_string = {
    {NewEntryType::User, "User"},
    {NewEntryType::Computer, "Computer"},
    {NewEntryType::OU, "Organization Unit"},
    {NewEntryType::Group, "Group"},
};

// Class solely for emitting signals
class AdInterface: public QObject {
Q_OBJECT

public:

public slots:

signals:
    void entry_deleted(const QString &dn);
    void entry_changed(const QString &dn);
    void entry_created(const QString &dn);
    void user_moved(const QString &old_dn, const QString &new_dn , const QString &new_parent_dn);

private:

}; 

extern AdInterface ad_interface;
extern bool FAKE_AD;

bool ad_interface_login();
QList<QString> load_children(const QString &dn);
QMap<QString, QList<QString>> load_attributes(const QString &dn);
bool set_attribute(const QString &dn, const QString &attribute, const QString &value);
bool create_entry(const QString &name, const QString &dn, NewEntryType type);
void delete_entry(const QString &dn);
bool set_attribute(const QString &dn, const QString &attribute, const QString &value);
void move_user(const QString &user_dn, const QString &container_dn);
QString extract_name_from_dn(const QString &dn);
QString extract_parent_dn_from_dn(const QString &dn);

#endif /* AD_INTERFACE_H */
