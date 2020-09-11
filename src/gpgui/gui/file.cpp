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

#include "file.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <QFile>

// TODO: display errors in GUI?

QList<QString> file_get_children(const QString &path) {
    QList<QString> children;

    const QByteArray path_array = path.toLatin1();
    const char *path_cstr = path_array.constData();

    struct stat filestat;
    stat(path_cstr, &filestat);

    const bool is_dir = S_ISDIR(filestat.st_mode);
    if (is_dir) {
        DIR *dirp = opendir(path_cstr);

        struct dirent *child;
        while ((child = readdir(dirp)) != NULL) {
            const char *child_name_cstr = child->d_name;
            const QString child_name(child_name_cstr);


            const bool is_dot_path = (child_name == "." || child_name == "..");
            if (is_dot_path) {
                continue;
            }

            const QString child_path = path + "/" + child_name;

            children.append(child_path);
        }

        closedir(dirp);
    }

    return children;
}

QByteArray file_read(const QString &path) {
    QFile file(path);
    const bool open_success = file.open(QIODevice::ReadOnly | QIODevice::Text);

    if (open_success) {
        const QByteArray bytes = file.readAll();
        file.close();

        return bytes;
    } else  {
        printf("Error: read_file failed to open %s\n", qPrintable(path));

        return QByteArray();
    }
}

void file_write(const QString &path, const QByteArray &bytes) {
    QFile file(path);
    const bool open_success = file.open(QIODevice::QIODevice::WriteOnly | QIODevice::Truncate);

    if (open_success) {
        file.write(bytes);
        file.close();
    } else  {
        printf("Error: write_file failed to open %s\n", qPrintable(path));
    }
}
