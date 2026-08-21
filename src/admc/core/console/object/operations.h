#ifndef CORE_CONSOLE_OBJECT_OPERATIONS_H
#define CORE_CONSOLE_OBJECT_OPERATIONS_H

#include <QHash>
#include <QString>

#include "ad_object.h"

void object_update_move(AdInterface &ad,
                        const QHash<QString, AdObject> &object_map,
                        const QHash<QString, QString> &old_to_new_dn_map);

#endif  /* ifndef CORE_CONSOLE_OBJECT_OPERATIONS_H */
