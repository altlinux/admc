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

#include <QMessageBox>
#include <QString>
#include <QWidget>

QMessageBox *message_box_generic(const QMessageBox::Icon icon,
                                 const QString &title,
                                 const QString &text,
                                 QWidget *parent) {
    auto message_box = new QMessageBox(parent);
    message_box->setAttribute(Qt::WA_DeleteOnClose);
    message_box->setStandardButtons(QMessageBox::Ok);
    message_box->setWindowTitle(title);
    message_box->setText(text);
    message_box->setIcon(icon);

    message_box->open();

    return message_box;
}

QMessageBox *message_box_critical(QWidget *parent,
                                  const QString &title,
                                  const QString &text) {
    return message_box_generic(QMessageBox::Critical, title, text, parent);
}

QMessageBox *message_box_information(QWidget *parent,
                                     const QString &title,
                                     const QString &text) {
    return message_box_generic(QMessageBox::Information, title, text, parent);
}

QMessageBox *message_box_question(QWidget *parent,
                                  const QString &title,
                                  const QString &text) {
    return message_box_generic(QMessageBox::Question, title, text, parent);
}

QMessageBox *message_box_warning(QWidget *parent,
                                 const QString &title,
                                 const QString &text) {
    return message_box_generic(QMessageBox::Warning, title, text, parent);
}
