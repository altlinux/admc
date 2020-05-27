
#ifndef AD_PROXY_MODEL_H
#define AD_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class QModelIndex;
class AdModel;

// Filter out advanced entries when advanced view is off
class AdProxyModel final : public QSortFilterProxyModel {
public:
    explicit AdProxyModel(AdModel *model, QObject *parent);

    bool only_show_containers = false;

public slots:
    void on_advanced_view_toggled(bool checked);

private:
    bool advanced_view = false;

    bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;

};

#endif /* AD_PROXY_MODEL_H */
