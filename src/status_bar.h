
#ifndef STATUS_BAR_H
#define STATUS_BAR_H

#include "ad_interface.h"

#include <QStatusBar>

void status_bar_init(QStatusBar *status_bar);


// Shows names and values of attributes of the entry selected in contents view
class StatusBar final : public QStatusBar {
Q_OBJECT

public:
    explicit StatusBar();

private slots:
    void on_load_children_failed(const QString &dn, const QString &error_str);
    void on_load_attributes_failed(const QString &dn, const QString &error_str);

    void on_delete_entry_complete(const QString &dn);
    void on_set_attribute_complete(const QString &dn, const QString &attribute, const QString &old_value, const QString &value); 
    void on_create_entry_complete(const QString &dn, NewEntryType type);
    void on_move_user_complete(const QString &user_dn, const QString &container_dn, const QString &new_dn);
    void on_add_user_to_group_complete(const QString &group_dn, const QString &user_dn);

    void on_delete_entry_failed(const QString &dn, const QString &error_str);
    void on_set_attribute_failed(const QString &dn, const QString &attribute, const QString &old_value, const QString &value, const QString &error_str);
    void on_create_entry_failed(const QString &dn, NewEntryType type, const QString &error_str);
    void on_move_user_failed(const QString &user_dn, const QString &container_dn, const QString &new_dn, const QString &error_str);
    void on_add_user_to_group_failed(const QString &group_dn, const QString &user_dn, const QString &error_str);

};

#endif /* STATUS_BAR_H */
