
#ifndef AD_PROXY_MODEL_H
#define AD_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class QModelIndex;
class QAction;
class AdModel;

// TODO: only allow AdModel source models
// Filter out advanced entries when advanced view is off
// Connected to advanced view toggle in menubar
class AdProxyModel final : public QSortFilterProxyModel {
public:
    explicit AdProxyModel(AdModel *model, QAction *advanced_view_toggle, QWidget *parent);

    bool only_show_containers = false;

private slots:
    void on_advanced_view_toggled(bool checked);

private:
    bool advanced_view = false;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

};

#endif /* AD_PROXY_MODEL_H */
