/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2026 BaseALT Ltd.
 * Copyright (C) 2020-2025 Dmitry Degtyarev
 * Copyright (C) 2026 Artyom V. Poptsov
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

#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>

#include "adldap.h"
#include "core/globals.h"
#include "core/user.h"
#include "ui/attribute_edit/general_name_edit.h"
#include "ui/attribute_edit/photo_edit.h"
#include "ui/attribute_edit/string_edit.h"
#include "ui/attribute_edit/string_other_edit.h"
#include "ui/dialog/image_view.h"
#include "ui/message_box.h"
#include "ui/status.h"
#include "ui/widget/tab/general_user.h"
#include "ui/widget/tab/ui_general_user.h"

GeneralUserTab::GeneralUserTab(QList<AttributeEdit *> *edit_list, QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralUserTab();
    ui->setupUi(this);

    edit_list->append(create_edits());
    ui->photo_edit->installEventFilter(this);

    ui->change_photo_button->show();
    connect(ui->change_photo_button, &QPushButton::clicked,
            this, &GeneralUserTab::on_change_photo_button_clicked);
}

GeneralUserTab::GeneralUserTab(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::GeneralUserTab();
    ui->setupUi(this);

    m_edit_list = create_edits();

    ui->name_label->setVisible(false);
    ui->description_edit->setReadOnly(true);
    ui->first_name_edit->setReadOnly(true);
    ui->last_name_edit->setReadOnly(true);
    ui->display_name_edit->setReadOnly(true);
    ui->initials_edit->setReadOnly(true);
    ui->email_edit->setReadOnly(true);
    ui->office_edit->setReadOnly(true);
    ui->web_page_edit->setReadOnly(true);
    ui->telephone_edit->setReadOnly(true);
    ui->web_page_button->setVisible(false);
    ui->telephone_button->setVisible(false);
    ui->middle_name_edit->setReadOnly(true);
    ui->change_photo_button->hide();

    ui->photo_edit->installEventFilter(this);
}

void GeneralUserTab::on_change_photo_button_clicked() {
    QString file_name = QFileDialog::getOpenFileName(
        this,
        tr("Select an image"),
        QStandardPaths::writableLocation(QStandardPaths::HomeLocation),
        tr("Images (*.jpg)"));
    QString error;
    if (! user_thumbnail_photo_file_check(file_name, error)) {
        g_status->add_message(error, StatusType_Error);
        message_box_warning(this,
                            tr("User photo uploading error"),
                            error);
    } else {
        QPixmap photo(file_name);
        jpeg_photo_edit->set_thumbnail_photo(photo);
    }
}

bool GeneralUserTab::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (object == ui->photo_edit) {
            view_photo();
        }
        return true;
    }

    return QObject::eventFilter(object, event);
}

void GeneralUserTab::view_photo() {
    QPixmap photo = jpeg_photo_edit->get_thumbnail_photo();
    if (! photo.isNull()) {
        ImageViewDialog *dialog = new ImageViewDialog(photo, this);
        QString first_name = ui->first_name_edit->text();
        QString middle_name = ui->middle_name_edit->text();
        QString last_name = ui->last_name_edit->text();
        QString name = first_name + " " + middle_name + " " + last_name;
        name = name.trimmed();
        if (! name.isEmpty()) {
            dialog->setWindowTitle(tr("User photo") + ": " + name);
        } else {
            dialog->setWindowTitle(tr("User photo"));
        }
        dialog->open();
    }
}

void GeneralUserTab::update(AdInterface &ad, const AdObject &object) {
    AttributeEdit::load(m_edit_list, ad, object);
}

GeneralUserTab::~GeneralUserTab() {
    delete ui;
}

QList<AttributeEdit *> GeneralUserTab::create_edits() {
    auto name_edit = new GeneralNameEdit(ui->name_label, this);
    auto description_edit = new StringEdit(ui->description_edit, ATTRIBUTE_DESCRIPTION, this);
    auto first_name_edit = new StringEdit(ui->first_name_edit, ATTRIBUTE_FIRST_NAME, this);
    auto last_name_edit = new StringEdit(ui->last_name_edit, ATTRIBUTE_LAST_NAME, this);
    auto display_name_edit = new StringEdit(ui->display_name_edit, ATTRIBUTE_DISPLAY_NAME, this);
    auto initials_edit = new StringEdit(ui->initials_edit, ATTRIBUTE_INITIALS, this);
    auto mail_edit = new StringEdit(ui->email_edit, ATTRIBUTE_MAIL, this);
    auto office_edit = new StringEdit(ui->office_edit, ATTRIBUTE_OFFICE, this);
    auto middle_name_edit = new StringEdit(ui->middle_name_edit, ATTRIBUTE_MIDDLE_NAME, this);
    jpeg_photo_edit = new PhotoEdit(ui->photo_edit, this);

    auto telephone_edit = new StringOtherEdit(ui->telephone_edit, ui->telephone_button, ATTRIBUTE_TELEPHONE_NUMBER, ATTRIBUTE_TELEPHONE_NUMBER_OTHER, this);
    auto web_page_edit = new StringOtherEdit(ui->web_page_edit, ui->web_page_button, ATTRIBUTE_WWW_HOMEPAGE, ATTRIBUTE_WWW_HOMEPAGE_OTHER, this);

    QList<AttributeEdit *> edits_out = {
        name_edit,
        description_edit,
        first_name_edit,
        last_name_edit,
        display_name_edit,
        initials_edit,
        mail_edit,
        office_edit,
        telephone_edit,
        web_page_edit,
        middle_name_edit,
        jpeg_photo_edit
    };

    return edits_out;
}

void GeneralUserTab::retranslate_ui() {
    ui->retranslateUi(this);
    for (auto* widget : children()) {
        QEvent languageEvent(QEvent::LanguageChange);
        QCoreApplication::sendEvent(widget, &languageEvent);
    }
}

bool GeneralUserTab::event(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    return QObject::event(event);
}
