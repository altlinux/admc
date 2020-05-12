
#include "ad_filter.h"
#include "ad_model.h"

#include <QAction>

AdFilter::AdFilter(const QAction * const advanced_view_action, bool only_show_containers) {
    this->only_show_containers = only_show_containers;

    // On advanced view toggle, copy advanced view flag and invalidate filter
    connect(advanced_view_action, &QAction::toggled,
        [this](bool checked)
        {
            this->advanced_view = checked;
            this->invalidateFilter();
        });
}

bool AdFilter::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    auto index = sourceModel()->index(source_row, 0, source_parent);

    // Hide advanced view only entries if advanced view is OFF
    auto advanced_view_only = index.data(AdModel::Roles::AdvancedViewOnly).toBool();
    if (advanced_view_only && !this->advanced_view) {
        return false;
    }

    if (only_show_containers) {
        // Hide non-containers
        auto is_container = index.data(AdModel::Roles::IsContainer).toBool();
        if (!is_container) {
            return false;
        }
    }

    return true;
}
