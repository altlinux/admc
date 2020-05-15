
#include "ad_interface.h"
#include "constants.h"

extern "C" {
#include "active_directory.h"
}

// TODO: replace C active_directory.h with C++ version

// -----------------------------------------------------------------
// FAKE STUFF
// -----------------------------------------------------------------

bool FAKE_AD = false; 

QMap<QString, QList<QString>> fake_children;
QMap<QString, QMap<QString, QList<QString>>> fake_attributes;

void fake_ad_init() {
    fake_children[HEAD_DN] = {
        QString("CN=Users,") + HEAD_DN,
        QString("CN=Computers,") + HEAD_DN,
        QString("CN=A,") + HEAD_DN,
        QString("CN=B,") + HEAD_DN,
        QString("CN=C,") + HEAD_DN,
        QString("CN=D,") + HEAD_DN,
    };

    fake_attributes[HEAD_DN] = {
        {"name", {"domain"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };

    fake_attributes[QString("CN=Users,") + HEAD_DN] = {
        {"name", {"Users"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
        {"description", {"Users's description"}},
    };

    fake_attributes[QString("CN=Computers,") + HEAD_DN] = {
        {"name", {"Computers"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
        {"description", {"Computers's description"}},
    };

    fake_attributes[QString("CN=A,") + HEAD_DN] = {
        {"name", {"A"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
        {"description", {"A's description"}},
    };

    fake_attributes[QString("CN=B,") + HEAD_DN] = {
        {"name", {"B"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
        {"description", {"B's description"}},
    };

    fake_attributes[QString("CN=C,") + HEAD_DN] = {
        {"name", {"C"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };

    fake_attributes[QString("CN=D,") + HEAD_DN] = {
        {"name", {"D"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Container,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"TRUE"}},
    };

    fake_children[QString("CN=A,") + HEAD_DN] = {
        QString("CN=B's child,CN=B,") + HEAD_DN
    };
    fake_attributes[QString("CN=B's child,CN=B,") + HEAD_DN] = {
        {"name", {"B's child"}},
        {"objectClass", {"person"}},
        {"objectCategory", {"CN=Person,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

QList<QString> fake_load_children(QString &dn) {
    if (!fake_children.contains(dn)) {
        // NOTE: ok to have empty children for leaves
        fake_children[dn] = QList<QString>();
    }
    
    return fake_children[dn];
}

QMap<QString, QList<QString>> fake_load_attributes(QString &dn) {
    if (!fake_attributes.contains(dn)) {
        printf("load_attributes failed for %s, loading empty attributes\n", qPrintable(dn));
        fake_attributes[dn] = QMap<QString, QList<QString>>();
    }

    return fake_attributes[dn];
}

// NOTE: this is just for fake_create() functions
void fake_create_add_child(QString &dn, QString &parent) {
    if (!fake_children.contains(parent)) {
        fake_children[parent] = QList<QString>();
    }
    
    fake_children[parent].push_back(dn);
}

void fake_create_user(QString &dn, QString &parent, QString &name) {
    fake_create_add_child(dn, parent);

    fake_attributes[dn] = {
        {"name", {name}},
        {"objectClass", {"user"}},
        {"objectCategory", {"CN=User,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

void fake_create_computer(QString &dn, QString &parent, QString &name) {
    fake_create_add_child(dn, parent);

    fake_attributes[dn] = {
        {"name", {name}},
        {"objectClass", {"computer"}},
        {"objectCategory", {"CN=Computer,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

void fake_create_ou(QString &dn, QString &parent, QString &name) {
    fake_create_add_child(dn, parent);

    fake_attributes[dn] = {
        {"name", {name}},
        {"objectClass", {"Organizational Unit"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Organizational-Unit,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

void fake_create_group(QString &dn, QString &parent, QString &name) {
    fake_create_add_child(dn, parent);

    fake_attributes[dn] = {
        {"name", {name}},
        {"objectClass", {"group"}},
        {"objectClass", {"container"}},
        {"objectCategory", {"CN=Group,CN=Schema,CN=Configuration"}},
        {"showInAdvancedViewOnly", {"FALSE"}},
    };
}

// -----------------------------------------------------------------
// REAL STUFF
// -----------------------------------------------------------------


bool ad_interface_login() {
    if (FAKE_AD) {
        fake_ad_init();
        return true;
    }

    LDAP* ldap_connection = ad_login();
    if (ldap_connection == NULL) {
        printf("ad_login error: %s\n", ad_get_error());
        return false;
    } else {
        return true;
    }
}

// TODO: confirm that this encoding is ok
char *qstring_to_cstr(QString &qstr) {
    return qstr.toLatin1().data();
}

QList<QString> load_children(QString &dn) {
    if (FAKE_AD) {
        return fake_load_children(dn);
    }

    char *dn_cstr = qstring_to_cstr(dn);
    char **children_raw = ad_list(dn_cstr);

    // TODO: error check

    if (children_raw != NULL) {
        auto children = QList<QString>();

        for (int i = 0; children_raw[i] != NULL; i++) {
            auto child = QString(children_raw[i]);
            children.push_back(child);
        }

        for (int i = 0; children_raw[i] != NULL; i++) {
            free(children_raw[i]);
        }
        free(children_raw);

        return children;
    } else {
        return QList<QString>();
    }
}

QMap<QString, QList<QString>> load_attributes(QString &dn) {
    if (FAKE_AD) {
        return fake_load_attributes(dn);
    }

    // TODO: save original attributes ordering and load it like that into model

    char *dn_cstr = qstring_to_cstr(dn);
    char** attributes_raw = ad_get_attribute(dn_cstr, NULL);

    // TODO: handle errors

    if (attributes_raw != NULL) {
        auto attributes = QMap<QString, QList<QString>>();

        // Load attributes map
        // attributes_raw is in the form of:
        // char** array of {key, value, value, key, value ...}
        // transform it into:
        // map of {key => {value, value ...}, key => {value, value ...} ...}
        for (int i = 0; attributes_raw[i + 2] != NULL; i += 2) {
            auto attribute = QString(attributes_raw[i]);
            auto value = QString(attributes_raw[i + 1]);

            // Make values list if doesn't exist yet
            if (!attributes.contains(attribute)) {
                attributes[attribute] = QList<QString>();
            }

            attributes[attribute].push_back(value);
        }

        // Free attributes_raw
        for (int i = 0; attributes_raw[i] != NULL; i++) {
            free(attributes_raw[i]);
        }
        free(attributes_raw);

        return attributes;
    } else {
        return QMap<QString, QList<QString>>();
    }
}

bool set_attribute(QString &dn, QString &attribute, QString &value) {
    if (FAKE_AD) {
        fake_attributes[dn][attribute] = {value};

        return true;
    }

    char *dn_cstr = qstring_to_cstr(dn);
    char *attribute_cstr = qstring_to_cstr(attribute);
    char *value_cstr = qstring_to_cstr(value);

    // TODO: handle errors

    int result = ad_mod_replace(dn_cstr, attribute_cstr, value_cstr);

    if (result == AD_SUCCESS) {
        return true;
    } else {
        return false;
    }
}

// TODO: can probably make a create_anything() function with enum parameter
bool create_entry(QString &name, QString &dn, QString &parent_dn, NewEntryType type) {
    // TODO: handle errors

    if (FAKE_AD) {
        switch (type) {
            case User: {
                fake_create_user(dn, parent_dn, name);
                break;
            }
            case Computer: {
                fake_create_computer(dn, parent_dn, name);
                break;
            }
            case OU: {
                fake_create_ou(dn, parent_dn, name);
                break;
            }
            case Group: {
                fake_create_group(dn, parent_dn, name);
                break;
            }
        }

        return true;
    }

    char *name_cstr = qstring_to_cstr(name);
    char *parent_cstr = qstring_to_cstr(parent_dn);

    int result;

    switch (type) {
        case User: {
            result = ad_create_user(name_cstr, parent_cstr);
            break;
        }
        case Computer: {
            result = ad_create_computer(name_cstr, parent_cstr);
            break;
        }
        case OU: {
            result = ad_ou_create(name_cstr, parent_cstr);
            break;
        }
        case Group: {
            result = ad_group_create(name_cstr, parent_cstr);
            break;
        }
    }

    if (result == AD_SUCCESS) {
        return true;
    } else {
        return false;
    }
}
