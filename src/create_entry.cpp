
#include "create_entry.h"
#include "ad_interface.h"
#include "constants.h"

#include <QInputDialog>
#include <QString>

void create_entry_dialog(NewEntryType type) {
    // Open new user dialog and name of entry from it

    QString dialog_title = "TITLE";
    switch (type) {
        case NewEntryType::User: {
            dialog_title = "New user";
            break;
        }
        case NewEntryType::Computer: {
            dialog_title = "New computer";
            break;
        }
        case NewEntryType::OU: {
            dialog_title = "New organizational unit";
            break;
        }
        case NewEntryType::Group: {
            dialog_title = "New group";
            break;
        }
    }

    QString input_label = "LABEL";
    switch (type) {
        case NewEntryType::User: {
            input_label = "User name:";
            break;
        }
        case NewEntryType::Computer: {
            input_label = "Computer name:";
            break;
        }
        case NewEntryType::OU: {
            input_label = "OU name:";
            break;
        }
        case NewEntryType::Group: {
            input_label = "Group name:";
            break;
        }
    }

    bool ok;
    QString name = QInputDialog::getText(nullptr, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    // TODO: maybe expand tree to newly created entry?

    // Create user once dialog is complete
    if (ok && !name.isEmpty()) {
        // Attempt to create user in AD

        // NOTE: for now create entries in their appropriate containers
        // OU/groups go straight to head
        QString parent_dn;
        switch (type) {
            case NewEntryType::User: {
                parent_dn = QString("CN=Users,") + HEAD_DN;
                break;
            }
            case NewEntryType::Computer: {
                parent_dn = QString("CN=Computers,") + HEAD_DN;
                break;
            }
            case NewEntryType::OU: {
                parent_dn = QString(HEAD_DN);
                break;
            }
            case NewEntryType::Group: {
                parent_dn = QString(HEAD_DN);
                break;
            }
        }

        QString suffix;
        switch (type) {
            case NewEntryType::User: {
                suffix = "CN";
                break;
            }
            case NewEntryType::Computer: {
                suffix = "CN";
                break;
            }
            case NewEntryType::OU: {
                suffix = "OU";
                break;
            }
            case NewEntryType::Group: {
                suffix = "CN";
                break;
            }
        }

        const QString dn = suffix + "=" + name + "," + parent_dn;

        create_entry(name, dn, type);
    }
}

void create_user_dialog() {
    create_entry_dialog(NewEntryType::User);
}

void create_computer_dialog() {
    create_entry_dialog(NewEntryType::Computer);
}

void create_ou_dialog() {
    create_entry_dialog(NewEntryType::OU);
}

void create_group_dialog() {
    create_entry_dialog(NewEntryType::Group);
}
