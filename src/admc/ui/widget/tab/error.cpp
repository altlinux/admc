/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "ui/widget/tab/error.h"
#include "ui/widget/tab/ui_error.h"

ErrorTab::ErrorTab(QWidget *parent)
: QWidget(parent) {
    ui = new Ui::ErrorTab();
    ui->setupUi(this);
}

ErrorTab::~ErrorTab() {
    delete ui;
}

void ErrorTab::retranslate_ui() {
    ui->retranslateUi(this);
    for (auto* widget : children()) {
        QEvent languageEvent(QEvent::LanguageChange);
        QCoreApplication::sendEvent(widget, &languageEvent);
    }
}

bool ErrorTab::event(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        retranslate_ui();
    }
    return QObject::event(event);
}
