#include "adldap.h"
#include "linked_policies_widget.h"
#include "ui_linked_policies_widget.h"
#include "console_impls/item_type.h"
#include "console_impls/policy_impl.h"
#include "console_impls/policy_ou_impl.h"
#include "console_impls/policy_root_impl.h"
#include "console_widget/console_widget.h"
#include "console_widget/results_view.h"
#include "globals.h"
#include "settings.h"
#include "status.h"
#include "utils.h"
#include "icon_manager/icon_manager.h"
#include "drag_drop_links_model.h"

#include <QAction>
#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QModelIndex>
#include <QStandardItemModel>
#include <QTreeView>

LinkedPoliciesWidget::LinkedPoliciesWidget(ConsoleWidget *console_arg, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LinkedPoliciesWidget),
    console(console_arg)
{
    ui->setupUi(this);

    if (!console) {
        return;
    }

    auto remove_link_action = new QAction(tr("Remove link"), this);
    auto move_up_action = new QAction(tr("Move up"), this);
    auto move_down_action = new QAction(tr("Move down"), this);

    context_menu = new QMenu(this);
    context_menu->addAction(remove_link_action);
    context_menu->addAction(move_up_action);
    context_menu->addAction(move_down_action);

    model = new DragDropLinksModel(gplink, 0, LinkedPoliciesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
        {
            {LinkedPoliciesColumn_Order, tr("Order")},
            {LinkedPoliciesColumn_Name, tr("Name")},
            {LinkedPoliciesColumn_Enforced, tr("Enforced")},
            {LinkedPoliciesColumn_Disabled, tr("Disabled")},
        });

    ui->view->set_model(model);

    QHeaderView *detail_view_header = ui->view->detail_view()->header();

    detail_view_header->resizeSection(0, 50);
    detail_view_header->resizeSection(1, 300);
    detail_view_header->resizeSection(2, 100);
    detail_view_header->resizeSection(3, 100);
    detail_view_header->resizeSection(4, 500);

    ui->view->set_drag_drop_internal();
    ui->view->current_view()->setDragDropOverwriteMode(false);
    ui->view->current_view()->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->view->current_view()->setSelectionMode(QAbstractItemView::ExtendedSelection);

    const QVariant state = settings_get_variant(SETTING_policy_ou_results_state);
    ui->view->restore_state(state,
        {
            LinkedPoliciesColumn_Order,
            LinkedPoliciesColumn_Name,
            LinkedPoliciesColumn_Enforced,
            LinkedPoliciesColumn_Disabled,
        });

    connect(
        model, &QStandardItemModel::itemChanged,
        this, &LinkedPoliciesWidget::on_item_changed);
    connect(
        ui->view, &ResultsView::context_menu,
        this, &LinkedPoliciesWidget::open_context_menu);
    connect(
        remove_link_action, &QAction::triggered,
        this, &LinkedPoliciesWidget::remove_link);
    connect(
        move_up_action, &QAction::triggered,
        this, &LinkedPoliciesWidget::move_up);
    connect(
        move_down_action, &QAction::triggered,
        this, &LinkedPoliciesWidget::move_down);

    connect(model, &DragDropLinksModel::link_orders_changed, [this](const Gplink &gplink_arg) {
        AdInterface ad;
        if (ad_failed(ad, this)) {
            model->arrange_orders_from_gplink(gplink);
            return;
        }

        bool success = ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink_arg.to_string());
        if (!success) {
            model->arrange_orders_from_gplink(gplink);
            return;
        }

        gplink = gplink_arg;
        const QModelIndex scope_tree_ou_index = console->get_current_scope_item();
        update_ou_item_gplink_data(gplink.to_string(), scope_tree_ou_index, console);

        g_status->add_message(tr("Organizational unit ") + scope_tree_ou_index.data().toString() + tr("'s link orders have been succesfuly changed."),
                              StatusType_Success);

        emit gplink_changed(scope_tree_ou_index);
    });
}

LinkedPoliciesWidget::~LinkedPoliciesWidget()
{
    const QVariant state = ui->view->save_state();
    settings_set_variant(SETTING_policy_ou_results_state, state);

    delete ui;
}

void LinkedPoliciesWidget::update(const QModelIndex &ou_index) {
    const ItemType type = (ItemType) console_item_get_type(ou_index);
    if (type != ItemType_PolicyOU) {
        return;
    }

    ou_dn = ou_index.data(PolicyOURole_DN).toString();

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    gplink = [&]() {
        const AdObject object = ad.search_object(ou_dn);
        const QString gplink_string = object.get_string(ATTRIBUTE_GPLINK);
        const Gplink out = Gplink(gplink_string);

        return out;
    }();

    update_link_items();
    emit gplink_changed(ou_index);
}

void LinkedPoliciesWidget::on_item_changed(QStandardItem *item) {
    const LinkedPoliciesColumn column = (LinkedPoliciesColumn) item->column();
    if (!option_columns.contains(column)) {
        return;
    }

    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    const QModelIndex this_index = item->index();
    const QModelIndex index = this_index.siblingAtColumn(0);
    const QString gpo_dn = index.data(LinkedPoliciesRole_DN).toString();
    const GplinkOption option = column_to_option[column];
    const bool is_checked = (item->checkState() == Qt::Checked);

    gplink.set_option(gpo_dn, option, is_checked);

    const QString gplink_string = gplink.to_string();

    bool success = ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink_string);

    if (success) {
        change_policy_icon(gpo_dn, is_checked, option);
    }

    g_status->display_ad_messages(ad, this);

    update_link_items();

    const QModelIndex scope_tree_ou_index = console->get_current_scope_item();
    update_ou_item_gplink_data(gplink_string, scope_tree_ou_index, console);
    emit gplink_changed(scope_tree_ou_index);

    hide_busy_indicator();
}

void LinkedPoliciesWidget::open_context_menu(const QPoint &pos) {
    const QModelIndex index = ui->view->current_view()->indexAt(pos);
    if (!index.isValid()) {
        return;
    }

    const QPoint global_pos = ui->view->current_view()->mapToGlobal(pos);
    context_menu->popup(global_pos);
}

void LinkedPoliciesWidget::remove_link() {
    // NOTE: save gpo dn list before they are removed in
    // modify_gplink()
    const QList<QString> gpo_dn_list = [&]() {
        QList<QString> out;

        const QList<QModelIndex> selected = ui->view->get_selected_indexes();

        for (const QModelIndex &index : selected) {
            const QString gpo_dn = index.data(LinkedPoliciesRole_DN).toString();

            out.append(gpo_dn);
        }

        return out;
    }();

    auto modify_function = [](Gplink &gplink_arg, const QString &gpo) {
        gplink_arg.remove(gpo);
    };

    modify_gplink(modify_function);

    // Also remove gpo from OU in console
    const QModelIndex policy_root = get_policy_tree_root(console);
    if (policy_root.isValid()) {
        const QModelIndex ou_index = console->search_item(policy_root, PolicyOURole_DN, ou_dn, {ItemType_PolicyOU});

        if (ou_index.isValid()) {
            for (const QString &gpo_dn : gpo_dn_list) {
                const QModelIndex gpo_index = console->search_item(ou_index, PolicyRole_DN, gpo_dn, {ItemType_Policy});

                if (gpo_index.isValid()) {
                    console->delete_item(gpo_index);
                }
            }
        }
    }
}

void LinkedPoliciesWidget::move_up() {
    auto modify_function = [](Gplink &gplink_arg, const QString &gpo) {
        gplink_arg.move_up(gpo);
    };

    modify_gplink(modify_function);
}

void LinkedPoliciesWidget::move_down() {
    auto modify_function = [](Gplink &gplink_arg, const QString &gpo) {
        gplink_arg.move_down(gpo);
    };

    modify_gplink(modify_function);
}

void LinkedPoliciesWidget::update_link_items() {
    model->removeRows(0, model->rowCount());

    AdInterface ad;
    if (ad_failed(ad, ui->view)) {
        return;
    }

    const QList<AdObject> gpo_obj_list = gpo_object_list(ad);

    for (const AdObject &gpo_object : gpo_obj_list) {
        const QList<QStandardItem *> row = make_item_row(LinkedPoliciesColumn_COUNT);
        load_item_row(gpo_object, row);
        model->appendRow(row);
    }

    model->sort(LinkedPoliciesColumn_Order);
}

// Takes as argument function that modifies the gplink.
// Modify function accepts gplink as argument and gpo dn
// used in the operation.
void LinkedPoliciesWidget::modify_gplink(void (*modify_function)(Gplink &, const QString &)) {
    AdInterface ad;
    if (ad_failed(ad, this)) {
        return;
    }

    show_busy_indicator();

    const QList<QModelIndex> selected = ui->view->get_selected_indexes();

    for (const QModelIndex &index : selected) {
        const QString gpo_dn = index.data(LinkedPoliciesRole_DN).toString();

        modify_function(gplink, gpo_dn);
    }

    const QString gplink_string = gplink.to_string();
    ad.attribute_replace_string(ou_dn, ATTRIBUTE_GPLINK, gplink_string);

    g_status->display_ad_messages(ad, this);

    update_link_items();

    const QModelIndex scope_tree_ou_index = console->get_current_scope_item();
    update_ou_item_gplink_data(gplink_string, scope_tree_ou_index, console);
    emit gplink_changed(scope_tree_ou_index);

    hide_busy_indicator();
}

void LinkedPoliciesWidget::change_policy_icon(const QString &policy_dn, bool is_checked, GplinkOption option) {
    QModelIndex target_policy_index = get_ou_child_policy_index(console, console->get_current_scope_item(), policy_dn);
    if (!target_policy_index.isValid())
        return;

    if (option == GplinkOption_Disabled)
    {
        set_disabled_policy_icon(console->get_item(target_policy_index), is_checked);
    }
    else if (option == GplinkOption_Enforced)
    {
        set_enforced_policy_icon(console->get_item(target_policy_index), is_checked);
    }
}

QList<AdObject> LinkedPoliciesWidget::gpo_object_list(AdInterface &ad) {
    const QList<QString> gpo_dn_list = gplink.get_gpo_list();

    if (gpo_dn_list.isEmpty()) {
        return QList<AdObject>();
    }

    const QString base = g_adconfig->policies_dn();
    const SearchScope scope = SearchScope_Children;
    const QString filter = filter_dn_list(gpo_dn_list);
    const QList<QString> attributes = QList<QString>();

    const QHash<QString, AdObject> search_results = ad.search(base, scope, filter, attributes);

    const QList<AdObject> gpo_objects = search_results.values();

    return gpo_objects;
}

void LinkedPoliciesWidget::load_item_row(const AdObject &gpo_object, QList<QStandardItem*> row) {
    const QList<QString> gpo_dn_list = gplink.get_gpo_list();

    // NOTE: Gplink may contain invalid gpo's, in which
    // case there's no real object for the policy, but
    // we still show the broken object to the user so he
    // is aware of it.
    const bool gpo_is_valid = !gpo_object.is_empty();
    const QString gpo_dn = gpo_object.get_dn();

    const QString name = gpo_is_valid ? gpo_object.get_string(ATTRIBUTE_DISPLAY_NAME) :
                                        tr("Not found");

    const int index = gpo_dn_list.indexOf(gpo_object.get_dn()) + 1;
    row[LinkedPoliciesColumn_Order]->setData(index, Qt::DisplayRole);

    row[LinkedPoliciesColumn_Name]->setText(name);
    set_data_for_row(row, gpo_dn, LinkedPoliciesRole_DN);

    row[0]->setIcon(g_icon_manager->get_icon_for_type(ItemIconType_Policy_Link));

    for (const auto column : option_columns) {
        QStandardItem *item = row[column];
        item->setCheckable(true);

        const Qt::CheckState checkstate = [=]() {
            const GplinkOption option = column_to_option[column];
            const bool option_is_set = gplink.get_option(gpo_dn, option);
            if (option_is_set) {
                return Qt::Checked;
            } else {
                return Qt::Unchecked;
            }
        }();
        item->setCheckState(checkstate);
    }

    if (!gpo_is_valid) {
        row[LinkedPoliciesColumn_Name]->setIcon(g_icon_manager->get_object_icon("block-indicator"));

        for (QStandardItem *item : row) {
            item->setToolTip(tr("The GPO for this link could not be found. It maybe have been recently created and is being replicated or it could have been deleted."));
        }
    }
}
