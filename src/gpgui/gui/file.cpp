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

#include <libsmbclient.h>

// TODO: display errors in GUI? lots of errors to handle...
// TODO: also test

// TODO: not checking returns of some f-ns like stat()
// they can contain important errors

// TODO: maybe can use 1 smbc context for the whole tree of file_get_children() calls? not sure have to test

enum FileLocation {
    FileLocation_Local,
    FileLocation_Smb
};

void get_auth_data_fn(const char * pServer, const char * pShare, char * pWorkgroup, int maxLenWorkgroup, char * pUsername, int maxLenUsername, char * pPassword, int maxLenPassword) {

}

SMBCCTX *make_smbc_context() {
    static bool smbc_init_called = false;
    if (!smbc_init_called) {
        smbc_init_called = true;

        smbc_init(get_auth_data_fn, 0);
    }

    SMBCCTX *context = smbc_new_context();
    smbc_setOptionUseKerberos(context, true);
    smbc_setOptionFallbackAfterKerberos(context, true);
    if (!smbc_init_context(context)) {
        smbc_free_context(context, 0);
        printf("Could not initialize smbc context\n");

        return nullptr;
    }
    smbc_set_context(context);

    return context;
}

FileLocation get_file_location(const QString &path) {
    if (path.startsWith("smb:")) {
        return FileLocation_Smb;
    } else {
        return FileLocation_Local;
    }
}

QList<QString> file_get_children(const QString &path) {
    const QByteArray path_bytes = path.toUtf8();
    const char *path_cstr = path_bytes.constData();

    QList<QString> children;

    const FileLocation location = get_file_location(path);

    auto add_child =
    [path, &children](const char *child_name_cstr) {
        const QString child_name(child_name_cstr);

        const bool is_dot_path = (child_name == "." || child_name == "..");
        if (is_dot_path) {
            return;
        } else {
            const QString child_path = path + "/" + child_name;
            children.append(child_path);
        }
    };

    switch (location) {
        case FileLocation_Local: {
            struct stat filestat;
            stat(path_cstr, &filestat);

            const bool is_dir = S_ISDIR(filestat.st_mode);
            if (is_dir) {
                DIR *dirp = opendir(path_cstr);

                struct dirent *child;
                while ((child = readdir(dirp)) != NULL) {
                    add_child(child->d_name);
                }

                closedir(dirp);
            }

            break;
        }
        case FileLocation_Smb: {
            SMBCCTX *context = make_smbc_context();

            struct stat filestat;
            smbc_stat(path_cstr, &filestat);

            const bool is_dir = S_ISDIR(filestat.st_mode);
            if (is_dir) {
                const int dirp = smbc_opendir(path_cstr);

                struct smbc_dirent *child;
                while ((child = smbc_readdir(dirp)) != NULL) {
                    add_child(child->name);
                }

                smbc_closedir(dirp);
            }

            smbc_free_context(context, 1);

            break;
        }
    }

    return children;
}

QByteArray file_read(const QString &path) {
    const FileLocation location = get_file_location(path);

    switch (location) {
        case FileLocation_Local: {
            QFile file(path);
            const bool open_success = file.open(QIODevice::ReadOnly | QIODevice::Text);

            if (open_success) {
                const QByteArray bytes = file.readAll();
                file.close();

                return bytes;
            } else  {
                printf("Error: file_read failed to open %s\n", qPrintable(path));

                return QByteArray();
            }

            break;
        }
        case FileLocation_Smb: {
            SMBCCTX *context = make_smbc_context();

            const QByteArray path_bytes = path.toUtf8();
            const char *path_cstr = path_bytes.constData();
            
            const int file = smbc_open(path_cstr, O_RDONLY, 0);

            const bool open_success = (file > 0);
            if (open_success) {
                // TODO: max size???
                const size_t buffer_size = 100000;
                char buffer[buffer_size];

                smbc_ftruncate(file, 0);
                const ssize_t bytes_read = smbc_read(file, (void *)buffer, buffer_size);

                smbc_close(file);

                const QByteArray bytes(buffer, bytes_read);

                return bytes;
            } else {
                printf("Error: file_write failed to open file\n");
            }

            smbc_free_context(context, 1);

            break;
        }
    }

    return QByteArray();
}

void file_write(const QString &path, const QByteArray &bytes) {
    const FileLocation location = get_file_location(path);

    switch (location) {
        case FileLocation_Local: {
            QFile file(path);
            const bool open_success = file.open(QIODevice::QIODevice::WriteOnly | QIODevice::Truncate);

            if (open_success) {
                file.write(bytes);
                file.close();
            } else  {
                printf("Error: file_write failed to open file\n");
            }

            break;
        }
        case FileLocation_Smb: {
            SMBCCTX *context = make_smbc_context();

            const QByteArray path_bytes = path.toUtf8();
            const char *path_cstr = path_bytes.constData();

            const int file = smbc_open(path_cstr, O_WRONLY, 0);

            const bool open_success = (file > 0);
            if (open_success) {
                const char *new_contents = bytes.constData();

                smbc_ftruncate(file, 0);
                smbc_write(file, (void *)new_contents, bytes.size());

                smbc_close(file);
            } else {
                printf("Error: file_write failed to open file\n");
            }

            smbc_free_context(context, 1);

            break;
        }
    }
}
