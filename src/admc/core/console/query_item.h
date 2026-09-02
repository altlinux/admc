#ifndef QUERY_ITEM_H
#define QUERY_ITEM_H

#include <QModelIndex>

#include "console.h"

enum QueryItemRole {
    QueryItemRole_Description = MyConsoleRole_LAST + 1,
    QueryItemRole_Filter,
    QueryItemRole_FilterState,
    QueryItemRole_Base,
    QueryItemRole_ScopeIsChildren,
    QueryItemRole_IsRoot,

    QueryItemRole_LAST,
};

enum QueryColumn {
    QueryColumn_Name,
    QueryColumn_Description,

    QueryColumn_COUNT,
};

QString index_data_filter(const QModelIndex &index);
QString index_data_base(const QModelIndex &index);
bool index_data_scope_is_children(const QModelIndex &index);
QString index_data_query_name(const QModelIndex &index);
QString index_data_description(const QModelIndex &index);
QByteArray index_data_filter_state(const QModelIndex &index);

#endif  /* ifndef QUERY_ITEM_H */
