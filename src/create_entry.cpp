
#include "create_entry.h"
#include "ad_interface.h"
#include "ad_model.h"
#include "constants.h"

#include <QInputDialog>
#include <QString>

AdModel *admodel = NULL;

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

        bool success = create_entry(name, dn, type);

        if (success) {
            // Load entry to model if it's parent has already been fetched
            // If it hasn't been fetched, then this new entry will be loaded with all other children when the parent is fetched


            // TODO: for some reason doesnt work with expanded parent

            QList<QStandardItem *> items = admodel->findItems(parent_dn, Qt::MatchExactly | Qt::MatchRecursive, AdModel::Column::DN);

            if (items.size() > 0) {
                QStandardItem *dn_item = items[0];
                QModelIndex dn_index = dn_item->index();
                QModelIndex parent_index = dn_index.siblingAtColumn(0);
                QStandardItem *parent = admodel->itemFromIndex(parent_index);

                bool fetched_already = !admodel->canFetchMore(parent_index);
                if (fetched_already) {
                    load_and_add_row(parent, dn);
                }
            }
        } else {

        }
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

void create_entry_init(AdModel *admodel_in) {
    admodel = admodel_in;
}
