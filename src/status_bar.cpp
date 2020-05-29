
#include "status_bar.h"
#include "ad_interface.h"

StatusBar::StatusBar()
: QStatusBar()
{
    showMessage(tr("Ready"), 10 * 1000);

    // Connect signals
    connect(
        &ad_interface, &AdInterface::load_children_failed,
        this, &StatusBar::on_load_children_failed);
    connect(
        &ad_interface, &AdInterface::load_attributes_failed,
        this, &StatusBar::on_load_attributes_failed);

    connect(
        &ad_interface, &AdInterface::create_entry_complete,
        this, &StatusBar::on_create_entry_complete);
    connect(
        &ad_interface, &AdInterface::set_attribute_complete,
        this, &StatusBar::on_set_attribute_complete);
    connect(
        &ad_interface, &AdInterface::create_entry_complete,
        this, &StatusBar::on_create_entry_complete);
    connect(
        &ad_interface, &AdInterface::move_user_complete,
        this, &StatusBar::on_move_user_complete);
    connect(
        &ad_interface, &AdInterface::add_user_to_group_complete,
        this, &StatusBar::on_add_user_to_group_complete);

    connect(
        &ad_interface, &AdInterface::delete_entry_failed,
        this, &StatusBar::on_delete_entry_failed);
    connect(
        &ad_interface, &AdInterface::set_attribute_failed,
        this, &StatusBar::on_set_attribute_failed);
    connect(
        &ad_interface, &AdInterface::create_entry_failed,
        this, &StatusBar::on_create_entry_failed);
    connect(
        &ad_interface, &AdInterface::move_user_failed,
        this, &StatusBar::on_move_user_failed);
    connect(
        &ad_interface, &AdInterface::add_user_to_group_failed,
        this, &StatusBar::on_add_user_to_group_failed);
}

void StatusBar::on_load_children_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to load children of \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}
void StatusBar::on_load_attributes_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to load attributes of \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}

void StatusBar::on_delete_entry_complete(const QString &dn) {
    QString msg = QString("Deleted entry \"%1\"").arg(dn);

    showMessage(msg);
}
void StatusBar::on_set_attribute_complete(const QString &dn, const QString &attribute, const QString &old_value, const QString &value) {
    QString msg = QString("Changed attribute \"%1\" of \"%2\" from \"%3\" to \"%4\"").arg(attribute, dn, old_value, value);

    showMessage(msg);
}
void StatusBar::on_create_entry_complete(const QString &dn, NewEntryType type) {
    QString type_str = new_entry_type_to_string[type];
    QString msg = QString("Created entry \"%1\" of type \"%2\"").arg(dn, type_str);

    showMessage(msg);
}
void StatusBar::on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn) {
    QString msg = QString("Moved entry \"%1\" to \"%2\"").arg(user_dn).arg(new_dn);

    showMessage(msg);
}
void StatusBar::on_add_user_to_group_complete(const QString &group_dn, const QString &user_dn) {
    QString msg = QString("Added user \"%1\" to group \"%2\"").arg(user_dn, group_dn);

    showMessage(msg);
}

// TODO: how to do translation of error messages coming from english-only lib???
// Probably will end up not using raw error strings anyway, just process error codes to generate localized string
void StatusBar::on_delete_entry_failed(const QString &dn, const QString &error_str) {
    QString msg = QString("Failed to delete entry \"%1\". Error: \"%2\"").arg(dn, error_str);

    showMessage(msg);
}
void StatusBar::on_set_attribute_failed(const QString &dn, const QString &attribute, const QString &old_value, const QString &value, const QString &error_str) {
    QString msg = QString("Failed to change attribute \"%1\" of entry \"%2\" from \"%3\" to \"%4\". Error: \"%5\"").arg(attribute, dn, old_value, value, error_str);

    showMessage(msg);
}
void StatusBar::on_create_entry_failed(const QString &dn, NewEntryType type, const QString &error_str) {
    QString type_str = new_entry_type_to_string[type];
    QString msg = QString("Failed to create entry \"%1\" of type \"%2\". Error: \"%3\"").arg(dn, type_str, error_str);

    showMessage(msg);
}
void StatusBar::on_move_user_failed(const QString &user_dn, const QString &container_dn, const QString &new_dn, const QString &error_str) {
    QString msg = QString("Failed to move user \"%1\". Error: \"%2\"").arg(user_dn, error_str);

    showMessage(msg);
}
void StatusBar::on_add_user_to_group_failed(const QString &group_dn, const QString &user_dn, const QString &error_str) {
    QString msg = QString("Failed to add user \"%1\" to group \"%2\". Error: \"%3\"").arg(user_dn, group_dn, error_str);

    showMessage(msg);
}
