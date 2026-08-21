#ifndef DRAG_N_DROP_H
#define DRAG_N_DROP_H

class QModelIndex;

namespace ObjectDragDrop {
enum DropType {
    DropType_Move,
    DropType_AddToGroup,
    DropType_None
};

DropType console_object_get_drop_type(const QModelIndex &dropped, const QModelIndex &target);
}

#endif // DRAG_N_DROP_H
