
#include "create_entry_dialog.h"
#include "ad_interface.h"
#include "constants.h"

#include <QInputDialog>
#include <QString>

void create_entry_dialog(NewEntryType type) {
    // Open new user dialog and name of entry from it

    const QMap<NewEntryType, QString> new_entry_type_to_string = {
        {NewEntryType::User, "User"},
        {NewEntryType::Computer, "Computer"},
        {NewEntryType::OU, "Organization Unit"},
        {NewEntryType::Group, "Group"},
    };
    QString type_string = new_entry_type_to_string[type];
    QString dialog_title = "New " + type_string;
    QString input_label = type_string + " name";

    bool ok;
    QString name = QInputDialog::getText(nullptr, dialog_title, input_label, QLineEdit::Normal, "", &ok);

    // TODO: maybe expand tree to newly created entry?

    // Create user once dialog is complete
    if (ok && !name.isEmpty()) {
        // Attempt to create user in AD

        // NOTE: for now create entries in their appropriate containers
        // OU/groups go straight to head
        const QMap<NewEntryType, QString> new_entry_type_to_parent = {
            {NewEntryType::User, QString("CN=Users,") + HEAD_DN},
            {NewEntryType::Computer, QString("CN=Computers,") + HEAD_DN},
            {NewEntryType::OU, QString(HEAD_DN)},
            {NewEntryType::Group, QString(HEAD_DN)},
        };
        QString parent_dn = new_entry_type_to_parent[type];

        const QMap<NewEntryType, QString> new_entry_type_to_suffix = {
            {NewEntryType::User, "CN"},
            {NewEntryType::Computer, "CN"},
            {NewEntryType::OU, "OU"},
            {NewEntryType::Group, "CN"},
        };
        QString suffix = new_entry_type_to_suffix[type];

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
