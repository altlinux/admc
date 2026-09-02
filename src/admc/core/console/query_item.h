#ifndef QUERY_ITEM_H
#define QUERY_ITEM_H

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

#endif  /* ifndef QUERY_ITEM_H */
