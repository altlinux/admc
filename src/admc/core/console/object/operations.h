#ifndef CORE_CONSOLE_OBJECT_OPERATIONS_H
#define CORE_CONSOLE_OBJECT_OPERATIONS_H

#include <QHash>
#include <QList>
#include <QString>

#include "ad_interface.h"
#include "ad_object.h"

void object_update_move(AdInterface &ad,
                        const QHash<QString, AdObject> &object_map,
                        const QHash<QString, QString> &old_to_new_dn_map);
void object_update_delete(AdInterface &ad,
                          const QString &object_class,
                          const QString &target_dn);
QList<AdObject> object_search_all(AdInterface &ad,
                                  const QList<QString> &dn_list);

#endif  /* ifndef CORE_CONSOLE_OBJECT_OPERATIONS_H */
