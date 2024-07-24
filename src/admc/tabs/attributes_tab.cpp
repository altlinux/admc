/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2022 BaseALT Ltd.
 * Copyright (C) 2020-2022 Dmitry Degtyarev
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
#include "tabs/ui_attributes_tab.h"

#include "adldap.h"
#include "attribute_dialogs/attribute_dialog.h"
#include "globals.h"
#include "settings.h"
#include "tabs/attributes_tab_filter_menu.h"
#include "tabs/attributes_tab_proxy.h"
#include "utils.h"

#include <QAction>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>
#include <QStandardItemModel>

QString attribute_type_display_string(const AttributeType type);

AttributesTab::AttributesTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::AttributesTab();
    ui->setupUi(this);

    auto tab_edit = new AttributesTabEdit(ui->view, ui->filter_button, ui->edit_button,
                                          ui->view_button, ui->load_optional_attrs_button, this);
    ui->view->setUniformRowHeights(true);

    edit_list->append({
        tab_edit,
    });
}

AttributesTabEdit::AttributesTabEdit(QTreeView *view_arg, QPushButton *filter_button_arg, QPushButton *edit_button_arg,
                                     QPushButton *view_button_arg, QPushButton *load_optional_attrs_button_arg, QObject *parent) : AttributeEdit(parent) {
    view = view_arg;
    filter_button = filter_button_arg;
    edit_button = edit_button_arg;
    view_button = view_button_arg;
    load_optional_attrs_button = load_optional_attrs_button_arg;

    model = new QStandardItemModel(0, AttributesColumn_COUNT, this);
    set_horizontal_header_labels_from_map(model,
        {
            {AttributesColumn_Name, tr("Name")},
            {AttributesColumn_Value, tr("Value")},
            {AttributesColumn_Type, tr("Type")},
        });

    auto filter_menu = new AttributesTabFilterMenu(view);

    proxy = new AttributesTabProxy(filter_menu, this);

    proxy->setSourceModel(model);
    view->setModel(proxy);

    filter_button->setMenu(filter_menu);

    enable_widget_on_selection(edit_button, view);

    settings_restore_header_state(SETTING_attributes_tab_header_state, view->header());

    const QHash<QString, QVariant> state = settings_get_variant(SETTING_attributes_tab_header_state).toHash();

    // This is the default sort, overriden by saved
    // sort when state is restored
    view->sortByColumn(AttributesColumn_Name, Qt::AscendingOrder);

    view->header()->restoreState(state["header"].toByteArray());

    QItemSelectionModel *selection_model = view->selectionModel();

    optional_attrs_values_is_loaded = settings_get_variant(SETTING_load_optional_attribute_values).toBool();
    load_optional_attrs_button->setVisible(!optional_attrs_values_is_loaded);

    connect(
        selection_model, &QItemSelectionModel::selectionChanged,
        this, &AttributesTabEdit::update_edit_and_view_buttons);
    update_edit_and_view_buttons();

    connect(
        view, &QAbstractItemView::doubleClicked,
        this, &AttributesTabEdit::on_double_click);
    connect(
        edit_button, &QAbstractButton::clicked,
        this, &AttributesTabEdit::edit_attribute);
    connect(
        view_button, &QAbstractButton::clicked,
        this, &AttributesTabEdit::view_attribute);
    connect(
        filter_menu, &AttributesTabFilterMenu::filter_changed,
        proxy, &AttributesTabProxy::invalidate);
    connect(
        load_optional_attrs_button, &QAbstractButton::clicked,
        this, &AttributesTabEdit::on_load_optional);
}

AttributesTab::~AttributesTab() {
    settings_set_variant(SETTING_attributes_tab_header_state, ui->view->header()->saveState());

    delete ui;
}

QList<QStandardItem *> AttributesTabEdit::get_selected_row() const {
    const QItemSelectionModel *selection_model = view->selectionModel();
    const QList<QModelIndex> selecteds = selection_model->selectedRows();

    if (selecteds.isEmpty()) {
        return QList<QStandardItem *>();
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

    return row;
}

void AttributesTabEdit::update_edit_and_view_buttons() {
    const QList<QStandardItem *> selected_row = get_selected_row();

    const bool no_selection = selected_row.isEmpty();
    if (no_selection) {
        edit_button->setVisible(true);
        edit_button->setEnabled(false);

        view_button->setVisible(false);
        view_button->setEnabled(false);
    } else {
        const QString attribute = selected_row[AttributesColumn_Name]->text();
        const bool read_only = g_adconfig->get_attribute_is_system_only(attribute);

        if (read_only) {
            edit_button->setVisible(false);
            edit_button->setEnabled(false);

            view_button->setVisible(true);
            view_button->setEnabled(true);
        } else {
            edit_button->setVisible(true);
            edit_button->setEnabled(true);

            view_button->setVisible(false);
            view_button->setEnabled(false);
        }
    }
}

void AttributesTabEdit::on_double_click() {
    const QList<QStandardItem *> selected_row = get_selected_row();
    const QString attribute = selected_row[AttributesColumn_Name]->text();
    const bool read_only = g_adconfig->get_attribute_is_system_only(attribute);

    if (read_only) {
        view_attribute();
    } else {
        edit_attribute();
    }
}

void AttributesTabEdit::view_attribute() {
    const bool read_only = true;
    AttributeDialog *dialog = get_attribute_dialog(read_only);
    if (dialog == nullptr) {
        return;
    }

    dialog->open();
}

void AttributesTabEdit::on_load_optional() {
    show_busy_indicator();

    AdInterface ad;
    if (!ad.is_connected()) {
        hide_busy_indicator();
        return;
    }

    load_optional_attribute_values(ad);
    current = original;
    reload_model();

    hide_busy_indicator();
}

void AttributesTabEdit::load_optional_attribute_values(AdInterface &ad) {
    // Chunks number is minimal query number (manually tested) to load objects
    // without errors. This value can be corrected after.
    const int attr_list_chunks_number = 5;
    int chunk_length = not_specified_optional_attributes.size()/attr_list_chunks_number;

    QSet<QString> optional_set_attrs;

    for (int from = 0; from < not_specified_optional_attributes.size(); from += chunk_length) {
        if (not_specified_optional_attributes.size() - from < chunk_length) {
            chunk_length = not_specified_optional_attributes.size() - from;
        }
        const QList<QString> attr_list_chunk = not_specified_optional_attributes.mid(from, chunk_length);
        const AdObject optional_attrs_object = ad.search_object(object_dn, attr_list_chunk);
        for (const QString &attribute : attr_list_chunk) {
            QList<QByteArray> values = optional_attrs_object.get_values(attribute);
            original[attribute] = values;
            if (!values.isEmpty()) {
                optional_set_attrs << attribute;
            }
        }
    }
    optional_attrs_values_is_loaded = true;
    proxy->update_set_attributes(optional_set_attrs);
}

void AttributesTabEdit::edit_attribute() {
    const bool read_only = false;
    AttributeDialog *dialog = get_attribute_dialog(read_only);
    if (dialog == nullptr) {
        return;
    }

    dialog->open();

    connect(
        dialog, &QDialog::accepted,
        this,
        [this, dialog]() {
            const QList<QStandardItem *> row = get_selected_row();

            if (row.isEmpty()) {
                return;
            }

            const QList<QByteArray> new_value_list = dialog->get_value_list();
            const QString attribute = dialog->get_attribute();

            current[attribute] = new_value_list;
            load_row(row, attribute, new_value_list);

            // TODO: fix
            emit edited();
        });
}

void AttributesTabEdit::load(AdInterface &ad, const AdObject &object) {
    UNUSED_ARG(ad);

    original.clear();
    object_dn = object.get_dn();

    for (auto attribute : object.attributes()) {
        original[attribute] = object.get_values(attribute);
    }

    const QList<QString> object_classes = object.get_strings(ATTRIBUTE_OBJECT_CLASS);
    const QList<QString> optional_attributes = g_adconfig->get_optional_attributes(object_classes);

    for (const QString &attribute : optional_attributes) {
        if (!original.contains(attribute)) {
            not_specified_optional_attributes << attribute;
        }
    }

    proxy->load(object);

    if (optional_attrs_values_is_loaded) {
        load_optional_attribute_values(ad);
    }
    else {
        for (const QString &attribute : not_specified_optional_attributes) {
            if (!original.contains(attribute)) {
                original[attribute] = QList<QByteArray>();
            }
        }
    }

    reload_model();
    current = original;
}

bool AttributesTabEdit::apply(AdInterface &ad, const QString &target) const {
    bool total_success = true;

    for (const QString &attribute : current.keys()) {
        const QList<QByteArray> current_values = current[attribute];
        const QList<QByteArray> original_values = original[attribute];

        if (current_values != original_values) {
            const bool success = ad.attribute_replace_values(target, attribute, current_values);
            if (!success) {
                total_success = false;
            }
        }
    }

    return total_success;
}

void AttributesTabEdit::load_row(const QList<QStandardItem *> &row, const QString &attribute, const QList<QByteArray> &values) {
    const QString display_values = attribute_display_values(attribute, values, g_adconfig);
    const AttributeType type = g_adconfig->get_attribute_type(attribute);
    const QString type_display = attribute_type_display_string(type);

    row[AttributesColumn_Name]->setText(attribute);
    row[AttributesColumn_Value]->setText(display_values);
    row[AttributesColumn_Type]->setText(type_display);
}

// Return an appropriate attribute dialog for currently
// selected attribute row.
AttributeDialog *AttributesTabEdit::get_attribute_dialog(const bool read_only) {
    const QList<QStandardItem *> row = get_selected_row();

    if (row.isEmpty()) {
        return nullptr;
    }

    const QString attribute = row[AttributesColumn_Name]->text();
    const QList<QByteArray> value_list = current[attribute];
    const bool single_valued = g_adconfig->get_attribute_is_single_valued(attribute);

    AttributeDialog *dialog = AttributeDialog::make(attribute, value_list, read_only, single_valued, view);

    return dialog;
}

void AttributesTabEdit::reload_model() {
    model->removeRows(0, model->rowCount());

    for (auto attribute : original.keys()) {
        const QList<QStandardItem *> row = make_item_row(AttributesColumn_COUNT);
        const QList<QByteArray> values = original[attribute];

        model->appendRow(row);
        load_row(row, attribute, values);
    }
}
