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

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QFile>
#include <QXmlStreamReader>

XmlEditor::XmlEditor(const QString &path)
: QDialog()
{   
    setAttribute(Qt::WA_DeleteOnClose);
    resize(300, 600);

    const QString title_label_text = tr("Editing xml file:") + path;
    auto title_label = new QLabel(title_label_text);

    auto button_box = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Ok);

    const auto top_layout = new QVBoxLayout();
    setLayout(top_layout);
    top_layout->addWidget(title_label);
    top_layout->addWidget(button_box);

    connect(
        button_box, &QDialogButtonBox::accepted,
        this, &QDialog::accept);
    connect(
        button_box, &QDialogButtonBox::rejected,
        this, &QDialog::reject);
    
    QFile file(path);
    const bool open_success = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open_success) {
        printf("Failed to open xml file\n");
        return;
    }

    const QByteArray file_byte_array = file.readAll();
    const QString file_string(file_byte_array);

    QXmlStreamReader xml(file_string);

    while (!xml.atEnd()) {
        const QXmlStreamReader::TokenType token_type = xml.readNext();

        switch (token_type) {
            case QXmlStreamReader::StartElement: {
                if (xml.name() == "Properties") {
                    const QXmlStreamAttributes attributes = xml.attributes();

                    for (const QXmlStreamAttribute attribute : attributes) {
                        printf("%s=%s\n", qPrintable(attribute.name().toString()), qPrintable(attribute.value().toString()));
                    }
                }

                break;
            }
            default: {
                break;
            }
        }        
    }
    if (xml.hasError()) {

    }
}
