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

#ifndef UTILS_H
#define UTILS_H

class QAbstractItemModel;
class QAbstractItemView;
class QAbstractProxyModel;
class QString;
class QCheckBox;
class QStandardItem;
class QStandardItemModel;
class QMenu;
class QTreeView;
class QLineEdit;
class QPlainTextEdit;
class QPoint;
class QWidget;
class QModelIndex;
class QVariant;
class QIcon;
class AdInterface;
class AdObject;
class QPersistentModelIndex;
class ConsoleWidget;
class QMessageBox;
template <typename T>
class QList;
template <typename K, typename T>
class QMap;
template <typename K, typename T>
class QHash;

#define UNUSED_ARG(x) (void) (x)

#define debug_print(a, args...) printf("%s(%s:%d) " a, __func__, __FILE__, __LINE__, ##args)
#define trace(a, args...) debug_print(a "\n", ##args)

QList<QStandardItem *> make_item_row(const int count);

// Convenience f-n so that you can pass a mapping of
// column => label
// Columns not in the map get empty labels
void set_horizontal_header_labels_from_map(QStandardItemModel *model, const QMap<int, QString> &labels_map);

// Prohibits leading zeroes
void set_line_edit_to_decimal_numbers_only(QLineEdit *edit);

void set_line_edit_to_hex_numbers_only(QLineEdit *edit);

void set_line_edit_to_time_span_format(QLineEdit *edit);

void enable_widget_on_selection(QWidget *widget, QAbstractItemView *view);

void show_busy_indicator();
void hide_busy_indicator();

bool confirmation_dialog(const QString &text, QWidget *parent);

void set_data_for_row(const QList<QStandardItem *> &row, const QVariant &data, const int role);

// Wrappers over is_connected() that also open an error
// messagebox if failed to connect. You should generally use
// these in GUI code instead of is_connected().
bool ad_connected(const AdInterface &ad, QWidget *parent);
bool ad_failed(const AdInterface &ad, QWidget *parent);

// Filter that accepts only given classes
QString get_classes_filter(const QList<QString> &class_list);

// Filter that accepts only container classes
QString is_container_filter();

void limit_edit(QLineEdit *edit, const QString &attribute);
void limit_plain_text_edit(QPlainTextEdit *edit, const QString &attribute);

QList<QPersistentModelIndex> persistent_index_list(const QList<QModelIndex> &indexes);
QList<QModelIndex> normal_index_list(const QList<QPersistentModelIndex> &indexes);

QString advanced_features_filter(const QString &filter);

void dev_mode_search_results(QHash<QString, AdObject> &results, AdInterface &ad, const QString &base);

// NOTE: these f-ns replace QMessageBox static f-ns. The
// static f-ns use exec(), which block execution and makes
// testing a hassle. These f-ns use open().
QMessageBox *message_box_critical(QWidget *parent, const QString &title, const QString &text);
QMessageBox *message_box_information(QWidget *parent, const QString &title, const QString &text);
QMessageBox *message_box_question(QWidget *parent, const QString &title, const QString &text);
QMessageBox *message_box_warning(QWidget *parent, const QString &title, const QString &text);

QList<QString> index_list_to_dn_list(const QList<QModelIndex> &index_list, const int dn_role);
QList<QString> get_selected_dn_list(ConsoleWidget *console, const int type, const int dn_role);
QString get_selected_target_dn(ConsoleWidget *console, const int type, const int dn_role);

void center_widget(QWidget *widget);

// If base name is "New X", then this will generate a name
// "New X (n)" where this name won't conflict with any
// existing names. For example "New Folder (7)"
QString generate_new_name(const QList<QString> &existing_name_list, const QString &base_name);

QList<QString> variant_list_to_string_list(const QList<QVariant> &variant_list);
QList<QVariant> string_list_to_variant_list(const QList<QString> &string_list);

bool string_contains_bad_chars(const QString &string, const QString &bad_chars);

bool verify_object_name(const QString &name, QWidget *parent);

// Setup an autofill of from one line edit into
// another, so that when src is edited, input is copied
// into dest.
void setup_lineedit_autofill(QLineEdit *src, QLineEdit *dest);

// (first name + last name) -> full name
void setup_full_name_autofill(QLineEdit *first_name_edit, QLineEdit *last_name_edit, QLineEdit *middle_name_edit, QLineEdit *full_name_edit);

QString gpo_status_from_int(int status);

QString current_dc_dns_host_name(AdInterface &ad);

#endif /* UTILS_H */
