/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020 BaseALT Ltd.
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

#include "xml_editor.h"
#include "xml_edit.h"
#include "xml_string_edit.h"
#include "xml_bool_edit.h"
#include "xml_ubyte_edit.h"
#include "xml_attribute.h"
#include "file.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QXmlStreamReader>
#include <QDomDocument>
#include <QPushButton>

QList<XmlAttribute> XmlEditor::schema_attributes;
QHash<QString, XmlAttribute> XmlEditor::schema_attributes_by_name;

void XmlEditor::load_schema() {
    static bool loaded = false;
    if (loaded) {
        return;
    } else {
        loaded = true;
    }

    QFile file(":/shortcuts_xml_schema.xml");
    const bool open_success = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open_success) {
        printf("Failed to open xml file\n");
        return;
    }
    QDomDocument doc;
    doc.setContent(&file);
    file.close();

    const QDomNodeList attributes = doc.elementsByTagName("xs:attribute");
    for (int i = 0; i < attributes.size(); i++) {
        const QDomNode node = attributes.at(i);
        const XmlAttribute attribute(node);

        schema_attributes.append(attribute);
        schema_attributes_by_name.insert(attribute.name(), attribute);
    }
}

XmlEditor::XmlEditor(const QString &path_arg)
: QDialog()
{   
    path = path_arg;

    load_schema();

    setAttribute(Qt::WA_DeleteOnClose);
    resize(300, 600);

    const QString title_label_text = tr("Editing xml file:") + path;
    auto title_label = new QLabel(title_label_text);

    auto button_box = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Reset | QDialogButtonBox::Close);
    apply_button = button_box->button(QDialogButtonBox::Apply);
    reset_button = button_box->button(QDialogButtonBox::Reset);

    auto edits_layout = new QGridLayout();

    for (auto attribute : schema_attributes) {
        if (attribute.hidden()) {
            continue;
        }

        auto make_edit =
        [this, attribute]() -> XmlEdit * {
            switch (attribute.type()) {
                case XmlAttributeType_String:
                return new XmlStringEdit(attribute, this);

                case XmlAttributeType_Boolean:
                return new XmlBoolEdit(attribute, this);

                case XmlAttributeType_UnsignedByte:
                return new XmlUByteEdit(attribute, this);

                case XmlAttributeType_None:
                return nullptr;
            }

            return nullptr;
        };
        XmlEdit *edit = make_edit();
        edit->add_to_layout(edits_layout);

        edits.append(edit);

        connect(edit, &XmlEdit::edited,
            this, &XmlEditor::enable_buttons_if_changed);
    }

    reload();

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(title_label);
    top_layout->addLayout(edits_layout);
    top_layout->addWidget(button_box);

    connect(
        button_box->button(QDialogButtonBox::Apply), &QPushButton::clicked,
        this, &XmlEditor::apply);
    connect(
        button_box->button(QDialogButtonBox::Reset), &QPushButton::clicked,
        this, &XmlEditor::reload);
    connect(
        button_box->button(QDialogButtonBox::Close), &QPushButton::clicked,
        this, &QDialog::reject);

    // Disable apply/reset buttons
    enable_buttons_if_changed();
}

void XmlEditor::enable_buttons_if_changed() {
    auto get_changed =
    [this]() -> bool {
        for (auto edit : edits) {
            if (edit->changed()) {
                return true;
            }
        }

        return false;
    };
    const bool changed = get_changed();

    apply_button->setEnabled(changed);
    reset_button->setEnabled(changed);
}

void XmlEditor::reload() {
    const QByteArray original_content = file_read(path);
    QDomDocument doc;
    doc.setContent(original_content);
    
    for (auto edit : edits) {
        edit->load(doc);
    }
}

bool XmlEditor::apply() {
    bool all_verified = true;
    for (auto edit : edits) {
        const bool verified = edit->verify_input(this);
        if (!verified) {
            all_verified = false;
            break;
        }
    }

    if (!all_verified) {
        return false;
    }

    const QByteArray original_content = file_read(path);
    QDomDocument doc;
    doc.setContent(original_content);

    for (auto edit : edits) {
        if (edit->changed()) {
            edit->apply(&doc);
        }
    }

    const QByteArray edited_content = doc.toByteArray(4);
    file_write(path, edited_content);

    return true;
}
