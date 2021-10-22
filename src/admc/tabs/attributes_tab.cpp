/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tabs/attributes_tab.h"
#include "tabs/attributes_tab_p.h"
#include "tabs/ui_attributes_tab.h"

#include "adldap.h"
#include "editors/attribute_editor.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

#include <QStandardItemModel>
#include <QMenu>
#include <QAction>
#include <QHeaderView>

enum AttributesColumn {
    AttributesColumn_Name,
    AttributesColumn_Value,
    AttributesColumn_Type,
    AttributesColumn_COUNT,
};

QString attribute_type_display_string(const AttributeType type);

AttributesTab::AttributesTab() {
    ui = new Ui::AttributesTab();
    ui->setupUi(this);

    model = new QStandardItemModel(0, AttributesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
    {
        {AttributesColumn_Name, tr("Name")},
        {AttributesColumn_Value, tr("Value")},
        {AttributesColumn_Type, tr("Type")},
    });

    auto filter_menu = new AttributesFilterMenu(this);

    proxy = new AttributesTabProxy(filter_menu, this);

    proxy->setSourceModel(model);
    ui->view->setModel(proxy);

    ui->filter_button->setMenu(filter_menu);

    enable_widget_on_selection(ui->edit_button, ui->view);

    settings_restore_header_state(SETTING_attributes_tab_header_state, ui->view->header());

    const QHash<QString, QVariant> state = settings_get_variant(SETTING_attributes_tab_header_state).toHash();

    ui->view->header()->restoreState(state["header"].toByteArray());

    connect(
        ui->view, &QAbstractItemView::doubleClicked,
        this, &AttributesTab::edit_attribute);
    connect(
        ui->edit_button, &QAbstractButton::clicked,
        this, &AttributesTab::edit_attribute);
    connect(
        filter_menu, &AttributesFilterMenu::filter_changed,
        proxy, &AttributesTabProxy::invalidate);
}

AttributesTab::~AttributesTab() {
    settings_set_variant(SETTING_attributes_tab_header_state, ui->view->header()->saveState());

    delete ui;
}

void AttributesTab::edit_attribute() {
    const QItemSelectionModel *selection_model = ui->view->selectionModel();
    const QList<QModelIndex> selecteds = selection_model->selectedRows();

    if (selecteds.isEmpty()) {
        return;
    }

    const QModelIndex proxy_index = selecteds[0];
    const QModelIndex index = proxy->mapToSource(proxy_index);

    const QList<QStandardItem *> row = [this, index]() {
        QList<QStandardItem *> out;
        for (int col = 0; col < AttributesColumn_COUNT; col++) {
            const QModelIndex item_index = index.siblingAtColumn(col);
            QStandardItem *item = model->itemFromIndex(item_index);
            out.append(item);
        }
        return out;
    }();

    const QString attribute = row[AttributesColumn_Name]->text();
    const QList<QByteArray> values = current[attribute];

    AttributeEditor *editor = AttributeEditor::make(attribute, values, this);
    if (editor != nullptr) {
        connect(
            editor, &QDialog::accepted,
            [this, editor, attribute, row]() {
                const QList<QByteArray> new_values = editor->get_new_values();

                current[attribute] = new_values;
                load_row(row, attribute, new_values);

                emit edited();
            });

        editor->open();
    } else {
        message_box_critical(this, tr("Error"), tr("No editor is available for this attribute type."));
    }
}

void AttributesTab::load(AdInterface &ad, const AdObject &object) {
    original.clear();

    for (auto attribute : object.attributes()) {
        original[attribute] = object.get_values(attribute);
    }

    // Add attributes without values
    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    const QList<QString> optional_attributes = g_adconfig->get_optional_attributes(object_classes);
    for (const QString &attribute : optional_attributes) {
        if (!original.contains(attribute)) {
            original[attribute] = QList<QByteArray>();
        }
    }

    current = original;

    proxy->load(object);

    model->removeRows(0, model->rowCount());

    for (auto attribute : original.keys()) {
        const QList<QStandardItem *> row = make_item_row(AttributesColumn_COUNT);
        const QList<QByteArray> values = original[attribute];

        model->appendRow(row);
        load_row(row, attribute, values);
    }

    ui->view->sortByColumn(AttributesColumn_Name, Qt::AscendingOrder);
}

bool AttributesTab::apply(AdInterface &ad, const QString &target) {
    bool total_success = true;

    for (const QString &attribute : current.keys()) {
        const QList<QByteArray> current_values = current[attribute];
        const QList<QByteArray> original_values = original[attribute];

        if (current_values != original_values) {
            const bool success = ad.attribute_replace_values(target, attribute, current_values);
            if (success) {
                original[attribute] = current_values;
            } else {
                total_success = false;
            }
        }
    }

    return total_success;
}

void AttributesTab::load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values) {
    const QString display_values = attribute_display_values(attribute, values, g_adconfig);
    const AttributeType type = g_adconfig->get_attribute_type(attribute);
    const QString type_display = attribute_type_display_string(type);

    row[AttributesColumn_Name]->setText(attribute);
    row[AttributesColumn_Value]->setText(display_values);
    row[AttributesColumn_Type]->setText(type_display);
}

AttributesFilterMenu::AttributesFilterMenu(QWidget *parent)
: QMenu(parent) {
    const QList<QVariant> state = settings_get_variant(SETTING_attributes_tab_filter_state).toList();

    auto add_filter_action = [&](const QString text, const AttributeFilter filter) {
        QAction *action = addAction(text);
        action->setText(text);
        action->setObjectName(QString::number(filter));
        action->setCheckable(true);

        const bool is_checked = [&]() {
            if (filter < state.size()) {
                return state[filter].toBool();
            } else {
                return true;
            }
        }();
        action->setChecked(is_checked);

        action_map.insert(filter, action);

        connect(
            action, &QAction::toggled,
            this, &AttributesFilterMenu::filter_changed);
    };

    add_filter_action(tr("Unset"), AttributeFilter_Unset);
    add_filter_action(tr("Read-only"), AttributeFilter_ReadOnly);

    addSeparator();

    add_filter_action(tr("Mandatory"), AttributeFilter_Mandatory);
    add_filter_action(tr("Optional"), AttributeFilter_Optional);

    addSeparator();

    add_filter_action(tr("System-only"), AttributeFilter_SystemOnly);
    add_filter_action(tr("Constructed"), AttributeFilter_Constructed);
    add_filter_action(tr("Backlink"), AttributeFilter_Backlink);

    connect(
        action_map[AttributeFilter_ReadOnly], &QAction::toggled,
        this, &AttributesFilterMenu::on_read_only_changed);
    on_read_only_changed();
}

AttributesFilterMenu::~AttributesFilterMenu() {
    const QList<QVariant> state = [&]() {
        QList<QVariant> out;

        for (int fitler_i = 0; fitler_i < AttributeFilter_COUNT; fitler_i++) {
            const AttributeFilter filter = (AttributeFilter) fitler_i;
            const QAction *action = action_map[filter];
            const QVariant filter_state = QVariant(action->isChecked());

            out.append(filter_state);
        }

        return out;
    }();

    settings_set_variant(SETTING_attributes_tab_filter_state, state);
}

void AttributesFilterMenu::on_read_only_changed() {
    const bool read_only_is_enabled = action_map[AttributeFilter_ReadOnly]->isChecked();

    const QList<AttributeFilter> read_only_sub_filters = {
        AttributeFilter_SystemOnly,
        AttributeFilter_Constructed,
        AttributeFilter_Backlink,
    };

    for (const AttributeFilter &filter : read_only_sub_filters) {
        action_map[filter]->setEnabled(read_only_is_enabled);
        
        // Turning off read only turns off the sub read only
        // filters. Note that turning ON read only doesn't do
        // the opposite.
        if (!read_only_is_enabled) {
            action_map[filter]->setChecked(false);
        }
    }
}

bool AttributesFilterMenu::filter_is_enabled(const AttributeFilter filter) const {
    return action_map[filter]->isChecked();
}

AttributesTabProxy::AttributesTabProxy(AttributesFilterMenu *filter_menu_arg, QObject *parent)
: QSortFilterProxyModel(parent) {
    filter_menu = filter_menu_arg;
}

void AttributesTabProxy::load(const AdObject &object) {
    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    mandatory_attributes = g_adconfig->get_mandatory_attributes(object_classes).toSet();
    optional_attributes = g_adconfig->get_optional_attributes(object_classes).toSet();

    set_attributes = object.attributes().toSet();
}

bool AttributesTabProxy::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const {
    auto source = sourceModel();
    const QString attribute = source->index(source_row, AttributesColumn_Name, source_parent).data().toString();

    const bool system_only = g_adconfig->get_attribute_is_system_only(attribute);
    const bool unset = !set_attributes.contains(attribute);
    const bool mandatory = mandatory_attributes.contains(attribute);
    const bool optional = optional_attributes.contains(attribute);

    if (!filter_menu->filter_is_enabled(AttributeFilter_Unset) && unset) {
        return false;
    }

    if (!filter_menu->filter_is_enabled(AttributeFilter_Mandatory) && mandatory) {
        return false;
    }

    if (!filter_menu->filter_is_enabled(AttributeFilter_Optional) && optional) {
        return false;
    }

    if (filter_menu->filter_is_enabled(AttributeFilter_ReadOnly) && system_only) {
        const bool constructed = g_adconfig->get_attribute_is_constructed(attribute);
        const bool backlink = g_adconfig->get_attribute_is_backlink(attribute);

        if (!filter_menu->filter_is_enabled(AttributeFilter_SystemOnly) && !constructed && !backlink) {
            return false;
        }

        if (!filter_menu->filter_is_enabled(AttributeFilter_Constructed) && constructed) {
            return false;
        }

        if (!filter_menu->filter_is_enabled(AttributeFilter_Backlink) && backlink) {
            return false;
        }
    }

    if (!filter_menu->filter_is_enabled(AttributeFilter_ReadOnly) && system_only) {
        return false;
    }

    return true;
}

QString attribute_type_display_string(const AttributeType type) {
    switch (type) {
        case AttributeType_Boolean: return AttributesTab::tr("Boolean");
        case AttributeType_Enumeration: return AttributesTab::tr("Enumeration");
        case AttributeType_Integer: return AttributesTab::tr("Integer");
        case AttributeType_LargeInteger: return AttributesTab::tr("Large Integer");
        case AttributeType_StringCase: return AttributesTab::tr("String Case");
        case AttributeType_IA5: return AttributesTab::tr("IA5");
        case AttributeType_NTSecDesc: return AttributesTab::tr("NT Security Descriptor");
        case AttributeType_Numeric: return AttributesTab::tr("Numeric");
        case AttributeType_ObjectIdentifier: return AttributesTab::tr("Object Identifier");
        case AttributeType_Octet: return AttributesTab::tr("Octet");
        case AttributeType_ReplicaLink: return AttributesTab::tr("Replica Link");
        case AttributeType_Printable: return AttributesTab::tr("Printable");
        case AttributeType_Sid: return AttributesTab::tr("SID");
        case AttributeType_Teletex: return AttributesTab::tr("Teletex");
        case AttributeType_Unicode: return AttributesTab::tr("Unicode");
        case AttributeType_UTCTime: return AttributesTab::tr("UTC Time");
        case AttributeType_GeneralizedTime: return AttributesTab::tr("Generalized Time");
        case AttributeType_DNString: return AttributesTab::tr("DN String");
        case AttributeType_DNBinary: return AttributesTab::tr("DN Binary");
        case AttributeType_DSDN: return AttributesTab::tr("Distinguished Name");
    }
    return QString();
}
