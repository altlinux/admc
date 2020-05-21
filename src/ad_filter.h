
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
    explicit AdFilter(AdModel *model, QAction *advanced_view_toggle);

    bool only_show_containers = false;

private slots:
    void on_advanced_view_toggled(bool checked);

private:
    bool advanced_view = false;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

};

#endif /* AD_FILTER_H */
