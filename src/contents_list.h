
#ifndef CONTENTS_VIEW_H
#define CONTENTS_VIEW_H

#include "entry_widget.h"

// Shows name, category and description of children of entry selected in containers view
class ContentsList : public EntryWidget {
Q_OBJECT

public:
    ContentsList(AdModel *model, QAction *advanced_view);

public slots:
    void on_selected_container_changed(const QModelIndex &source_index);

private:
    
};

#endif /* CONTENTS_VIEW_H */
