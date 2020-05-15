
#include "create_entry.h"
#include "ad_interface.h"
#include "ad_model.h"

#include <QInputDialog>
#include <QString>

AdModel *admodel = NULL;

void create_user_dialog() {
    // Open new user dialog and get username from it
    bool ok;
    auto username = QInputDialog::getText(nullptr, "New user", "User name:", QLineEdit::Normal, "", &ok);

    // TODO: maybe expand tree to newly created entry?

    // Create user once dialog is complete
    if (ok && !username.isEmpty()) {
        // Attempt to create user in AD
        bool success = create_entry(username, NewEntryType::User);

        if (success) {
            // Add user to model
            // NOTE: new users are appended to the "Users" container
            auto users_container_dn = get_users_container_dn();
            auto items = admodel->findItems(users_container_dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

            if (items.size() > 0) {
                auto users_item = items[0];

                auto user_dn = get_new_user_dn(username);
                load_and_add_row(users_item, user_dn);
            }
        } else {

        }
    }
}

void create_entry_init(AdModel *admodel_in) {
    admodel = admodel_in;
}
