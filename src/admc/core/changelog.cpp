/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2026 BaseALT Ltd.
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

#include <QCoreApplication>
#include <QFile>
#include <QLocale>
#include <QStandardPaths>
#include <QString>

/**
 * Read a change log file and return its contents.
 *
 * @return The change log file contents or an empty string on an error.
 */
QString changelog_read(const QLocale& locale) {
    QString changelog_file_name;
    if (locale.language() == QLocale::Russian) {
        changelog_file_name = "CHANGELOG_ru.txt";
    } else {
        changelog_file_name = "CHANGELOG.txt";
    }

#ifdef QT_DEBUG
    QString changelog_path =
        QString("%1/%2").arg(QCoreApplication::applicationDirPath(),
                             changelog_file_name);
#else
    QString changelog_path =
        QStandardPaths::locate(
            QStandardPaths::GenericDataLocation,
            QString("doc/admc-%1/%2").arg(ADMC_VERSION,
                                          changelog_file_name));
#endif

    if (changelog_path.isEmpty()) {
        return QString();
    }

    QFile file(changelog_path);

    const bool open_success = file.open(QIODevice::ReadOnly);
    if (!open_success) {
        qDebug() << "Failed to open changelog file";

        return QString();
    }

    QString changelog_text = file.readAll();

    file.close();

    // Remove forced word wrap contained in
    // CHANGELOG.txt so that resizing the dialog
    // expands text width (all wrapped lines start
    // with 2 spaces)
    changelog_text.replace("\n  ", " ");

    return changelog_text;
}
