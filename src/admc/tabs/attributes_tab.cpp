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

#include "adldap.h"
#include "editors/attribute_editor.h"
#include "globals.h"
#include "settings.h"
#include "utils.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTreeView>
#include <QVBoxLayout>
#include <QMenu>
#include <QAction>

enum AttributesColumn {
    AttributesColumn_Name,
    AttributesColumn_Value,
    AttributesColumn_Type,
    AttributesColumn_COUNT,
};

QString attribute_type_display_string(const AttributeType type);

AttributesTab::AttributesTab() {
    model = new QStandardItemModel(0, AttributesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
    {
        {AttributesColumn_Name, tr("Name")},
        {AttributesColumn_Value, tr("Value")},
        {AttributesColumn_Type, tr("Type")},
    });

    view = new QTreeView(this);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setSelectionBehavior(QAbstractItemView::SelectRows);
    view->setAllColumnsShowFocus(true);
    view->setSortingEnabled(true);

    proxy = new AttributesTabProxy(this);

    proxy->setSourceModel(model);
    view->setModel(proxy);

    auto edit_button = new QPushButton(tr("Edit..."));
    edit_button->setObjectName("edit_button");
    auto filter_button = new QPushButton(tr("Filter"));
    filter_button->setObjectName("filter_button");
    auto buttons = new QHBoxLayout();
    buttons->addWidget(edit_button);
    buttons->addStretch();
    buttons->addWidget(filter_button);

    filter_button->setMenu(new AttributesFilterMenu(this));

    const auto layout = new QVBoxLayout();
    setLayout(layout);
    layout->addWidget(view);
    layout->addLayout(buttons);

    g_settings->restore_header_state(VariantSetting_AttributesTabHeaderState, view->header());

    enable_widget_on_selection(edit_button, view);

    connect(
        view, &QAbstractItemView::doubleClicked,
        this, &AttributesTab::edit_attribute);
    connect(
        edit_button, &QAbstractButton::clicked,
        this, &AttributesTab::edit_attribute);
}

AttributesTab::~AttributesTab() {
    g_settings->save_header_state(VariantSetting_AttributesTabHeaderState, view->header());
}

void AttributesTab::edit_attribute() {
    const QItemSelectionModel *selection_model = view->selectionModel();
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

    view->sortByColumn(AttributesColumn_Name, Qt::AscendingOrder);
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
    auto add_filter_option = [&](const QString text, const BoolSetting setting) {
        QAction *action = addAction(text);
        action->setText(text);
        action->setObjectName(QString::number(setting));

        g_settings->connect_action_to_bool_setting(action, setting);

        action_map.insert(setting, action);
    };

    add_filter_option(tr("Unset"), BoolSetting_AttributeFilterUnset);
    add_filter_option(tr("Read-only"), BoolSetting_AttributeFilterReadOnly);

    addSeparator();

    add_filter_option(tr("Mandatory"), BoolSetting_AttributeFilterMandatory);
    add_filter_option(tr("Optional"), BoolSetting_AttributeFilterOptional);

    addSeparator();

    add_filter_option(tr("System-only"), BoolSetting_AttributeFilterSystemOnly);
    add_filter_option(tr("Constructed"), BoolSetting_AttributeFilterConstructed);
    add_filter_option(tr("Backlink"), BoolSetting_AttributeFilterBacklink);

    const BoolSettingSignal *read_only_signal = g_settings->get_bool_signal(BoolSetting_AttributeFilterReadOnly);
    connect(
        read_only_signal, &BoolSettingSignal::changed,
        this, &AttributesFilterMenu::on_read_only_changed);
    on_read_only_changed();
}

void AttributesFilterMenu::on_read_only_changed() {
    const bool read_only_is_enabled = g_settings->get_bool(BoolSetting_AttributeFilterReadOnly);

    const QList<BoolSetting> read_only_sub_filters = {
        BoolSetting_AttributeFilterSystemOnly,
        BoolSetting_AttributeFilterConstructed,
        BoolSetting_AttributeFilterBacklink,
    };

    for (const BoolSetting &setting : read_only_sub_filters) {
        action_map[setting]->setEnabled(read_only_is_enabled);
        action_map[setting]->setChecked(read_only_is_enabled);
    }
}

AttributesTabProxy::AttributesTabProxy(QObject *parent)
: QSortFilterProxyModel(parent) {
    const QList<BoolSetting> filter_setting_list = {
        BoolSetting_AttributeFilterUnset,
        BoolSetting_AttributeFilterReadOnly,
        BoolSetting_AttributeFilterMandatory,
        BoolSetting_AttributeFilterOptional,
        BoolSetting_AttributeFilterSystemOnly,
        BoolSetting_AttributeFilterConstructed,
        BoolSetting_AttributeFilterBacklink,
    };
    for (const BoolSetting &setting : filter_setting_list) {
        const BoolSettingSignal *signal = g_settings->get_bool_signal(setting);
        connect(
            signal, &BoolSettingSignal::changed,
            this, &AttributesTabProxy::invalidate);
    }
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

    if (!g_settings->get_bool(BoolSetting_AttributeFilterUnset) && unset) {
        return false;
    }

    if (!g_settings->get_bool(BoolSetting_AttributeFilterMandatory) && mandatory) {
        return false;
    }

    if (!g_settings->get_bool(BoolSetting_AttributeFilterOptional) && optional) {
        return false;
    }

    if (g_settings->get_bool(BoolSetting_AttributeFilterReadOnly) && system_only) {
        const bool constructed = g_adconfig->get_attribute_is_constructed(attribute);
        const bool backlink = g_adconfig->get_attribute_is_backlink(attribute);

        if (!g_settings->get_bool(BoolSetting_AttributeFilterSystemOnly) && !constructed && !backlink) {
            return false;
        }

        if (!g_settings->get_bool(BoolSetting_AttributeFilterConstructed) && constructed) {
            return false;
        }

        if (!g_settings->get_bool(BoolSetting_AttributeFilterBacklink) && backlink) {
            return false;
        }
    }

    if (!g_settings->get_bool(BoolSetting_AttributeFilterReadOnly) && system_only) {
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
