
#include "ad_proxy_model.h"
#include "ad_model.h"
#include "ad_interface.h"

#include <QAction>

AdProxyModel::AdProxyModel(AdModel *model, QAction *advanced_view_toggle, QWidget *parent)
: QSortFilterProxyModel(parent)
{
    this->setSourceModel(model);

    connect(advanced_view_toggle, &QAction::toggled,
        this, &AdProxyModel::on_advanced_view_toggled);
}

void AdProxyModel::on_advanced_view_toggled(bool checked) {
    // On advanced view toggle, copy advanced view flag and invalidate filter
    advanced_view = checked;
    this->invalidateFilter();
}

bool AdProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    const QModelIndex index = sourceModel()->index(source_row, 0, source_parent);

    // Hide advanced view only entries if advanced view is OFF
    const bool advanced_view_only = index.data(AdModel::Roles::AdvancedViewOnly).toBool();
    if (advanced_view_only && !this->advanced_view) {
        return false;
    }

    if (only_show_containers) {
        // Hide non-containers
        const bool is_container = index.data(AdModel::Roles::IsContainer).toBool();
        if (!is_container) {
            return false;
        }
    }

    return true;
}
