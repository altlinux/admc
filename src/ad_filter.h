
#ifndef AD_FILTER_H
#define AD_FILTER_H

#include <QSortFilterProxyModel>

class QModelIndex;
class QAction;
class AdModel;

// TODO: only allow AdModel source models
// Filter out advanced entries when advanced view is off
// Connected to advanced view toggle in menubar
class AdFilter : public QSortFilterProxyModel {
public:
    explicit AdFilter(const QAction * const advanced_view_action, bool only_show_containers = false);

private:
    bool advanced_view = false;
    bool only_show_containers;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
};

#endif /* AD_FILTER_H */
