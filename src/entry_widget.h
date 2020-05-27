
#ifndef ENTRY_WIDGET_H
#define ENTRY_WIDGET_H

#include "ad_model.h"

#include <QWidget>
#include <QMap>

class QTreeView;
class QAction;
class QLabel;
class AdModel;
class AdProxyModel;

// Shows names of AdModel as a tree
class EntryWidget : public QWidget {
Q_OBJECT

public:
    EntryWidget(AdModel *model);

    QString get_selected_dn() const;

private slots:
    void on_action_toggle_dn(bool checked);
    void on_context_menu_requested(const QPoint &pos);

protected:
    QTreeView *view = nullptr;
    AdProxyModel *proxy = nullptr;
    QLabel *label = nullptr;
    QMap<AdModel::Column, bool> column_hidden;
    
    void update_column_visibility();

};

#endif /* ENTRY_WIDGET_H */
