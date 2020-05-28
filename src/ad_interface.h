
#ifndef AD_INTERFACE_H
#define AD_INTERFACE_H

#include <QObject>
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
class AdInterface final : public QObject {
Q_OBJECT

public:

public slots:

signals:
    void delete_entry_complete(const QString &dn);
    void set_attribute_complete(const QString &dn, const QString &attribute, const QString &old_value, const QString &value);
    void create_entry_complete(const QString &dn, NewEntryType type);
    void move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn);

    void delete_entry_failed(const QString &dn, const QString &error_str);
    void set_attribute_failed(const QString &dn, const QString &attribute, const QString &old_value, const QString &value, const QString &error_str);
    void create_entry_failed(const QString &dn, NewEntryType type, const QString &error_str);
    void move_user_failed(const QString &user_dn, const QString &container_dn, const QString &new_dn, const QString &error_str);

private:

}; 

extern AdInterface ad_interface;
extern bool FAKE_AD;

bool ad_interface_login();
QList<QString> load_children(const QString &dn);
void load_attributes(const QString &dn);
QMap<QString, QList<QString>> get_attributes(const QString &dn);
QList<QString> get_attribute_multi(const QString &dn, const QString &attribute);
QString get_attribute(const QString &dn, const QString &attribute);
bool set_attribute(const QString &dn, const QString &attribute, const QString &value);
bool create_entry(const QString &name, const QString &dn, NewEntryType type);
void delete_entry(const QString &dn);
bool set_attribute(const QString &dn, const QString &attribute, const QString &value);
void move_user(const QString &user_dn, const QString &container_dn);
QString extract_name_from_dn(const QString &dn);
QString extract_parent_dn_from_dn(const QString &dn);

#endif /* AD_INTERFACE_H */
