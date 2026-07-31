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

#include "changelog_dialog.h"
#include "ui_changelog_dialog.h"

#include "config.h"
#include "core/changelog.h"
#include "core/settings.h"

#include <QCoreApplication>
#include <QDebug>
#include <QStandardPaths>
#include <QFile>

ChangelogDialog::ChangelogDialog(QWidget *parent)
: QDialog(parent) {
    ui = new Ui::ChangelogDialog();
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    const QLocale saved_locale = settings_get_current_locale();
    QString changelog_text = changelog_read(saved_locale);
    if (changelog_text.isEmpty()) {
        changelog_text = tr("Failed to open changelog file.");
    }
    ui->edit->setPlainText(changelog_text);
    settings_setup_dialog_geometry(SETTING_changelog_dialog_geometry, this);
}

ChangelogDialog::~ChangelogDialog() {
    delete ui;
}
