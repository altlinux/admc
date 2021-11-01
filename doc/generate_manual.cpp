/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2021 BaseALT Ltd.
 * Copyright (C) 2020-2021 Dmitry Degtyarev
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

/**
 * Generate manual files (.html and .qhp) from the manual in
 * wiki format (.wiki).
 */

#include <QtCore>

enum LineType {
    LineType_Normal,
    LineType_OL,
    LineType_UL,
    LineType_UL_2,
};

const QString OL_string = "# ";
const QString UL_string = "#* ";
const QString UL_2_string = "#** ";

const QString LI_start = "<li>";
const QString LI_end = "</li>";
const QString UL_start = "<ul>";
const QString UL_end = "</ul>";
const QString OL_start = "<ol>";
const QString OL_end = "</ol>";
const QString BR_string = "<br>";

QString get_section_title(const QList<QString> &section);
QString get_section_path(const QList<QString> &section);
LineType get_line_type(const QList<QString> &line_list);
QList<QString> generate_body(const QList<QString> &line_list_const);
QList<QString> generate_OL(QList<QString> &line_list);
QList<QString> generate_UL(QList<QString> &line_list);
QList<QString> generate_UL_2(QList<QString> &line_list);

int main(int argc, char **argv) {
    if (argc != 3) {
        qCritical() << "Incorrect arguments! Need: \"src_wiki\" \"dest_dir\"";

        return 1;
    }

    const QString wiki_file_path = argv[1];
    const QString dest_dir = argv[2];

    //
    // Read contents of wiki file
    //
    const QList<QString> line_list = [&]() {
        QFile wiki_file(wiki_file_path);

        const bool open_wiki_success = wiki_file.open(QIODevice::ReadOnly);

        if (!open_wiki_success) {
            qCritical() << "Failed to open wiki file:" << wiki_file_path;
            
            return QList<QString>();
        }

        const QString file_string = wiki_file.readAll();
        wiki_file.close();

        const QList<QString> out = file_string.split("\n");
        
        return out;
    }();

    if (line_list.isEmpty()) {
        qCritical() << "Failed to read wiki file";

        return 1;
    }

    // Split into sections
    const QList<QList<QString>> section_list = [&]() {
        QList<QList<QString>> out;

        int current_i = 0;

        auto find_next_section = [&](const int start_i) {
            for (int i = start_i; i < line_list.size(); i++) {
                const QString line = line_list[i];
                const bool section_start = line.startsWith("=");
                if (section_start) {
                    return i;
                }
            }

            return -1;
        };

        while (true) {
            const int section_start_i = find_next_section(current_i);
            if (section_start_i == -1) {
                break;
            }
            
            int section_end_i = find_next_section(section_start_i + 1);
            if (section_end_i == -1) {
                // Last section case
                section_end_i = line_list.size() - 1;
            }

            const int section_length = section_end_i - section_start_i;

            const QList<QString> section = line_list.mid(section_start_i, section_length);

            out.append(section);

            current_i = section_end_i;          
        }

        return out;
    }();

    //
    // Write section files (.html)
    //
    for (const QList<QString> &section : section_list) {
        const QList<QString> section_file_contents = [&]() {
            QList<QString> out;

            const QString section_name = get_section_title(section);

            out.append("<html>");
            out.append("<body>");
            out.append("<head>");
            out.append("<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">");
            out.append("</head>");

            out.append("<p>");
            out.append(QString("<h3>%1</h3>").arg(section_name));
            out.append("</p>");

            out.append("<p>");

            const QList<QString> body = generate_body(section);
            out.append(body);

            out.append("</p>");
            out.append("</body>");
            out.append("</html>");

            return out;
        }();

        const QString section_path = get_section_path(section);
        const QString section_file_path = QString("%1/%2.html").arg(dest_dir, section_path);
        QFile section_file(section_file_path);

        const bool open_section_success = section_file.open(QIODevice::WriteOnly | QIODevice::Text);

        if (!open_section_success) {
            qCritical() << "Failed to open section file:" << section_file_path;
            
            break;
        }

        section_file.write(section_file_contents.join("\n").toUtf8());        
        section_file.close();
    }

    //
    // Write .qhp file
    //
    const QList<QString> qhp_file_contents = [&]() {
        QList<QString> out;

        out.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        out.append("<QtHelpProject version=\"1.0\">");
        out.append("<namespace>alt.basealt.admc</namespace>");
        out.append("<virtualFolder>doc</virtualFolder>");
        out.append("<filterSection>");

        out.append("<toc>");

        int prev_level = 0;
        int section_start_count = 0;
        int section_end_count = 0;
        for (const QList<QString> &section : section_list) {
            const int level = [&]() {
                const QString section_line = section[0];

                int i = 0;
                while (i < section_line.size() && section_line[i] == "=") {
                    i++;
                }

                return i;
            }();

            const int level_diff = prev_level - level;
            if (level_diff >= 0) {
                for (int i = -1; i < level_diff; i++) {
                    out.append("</section>");
                    section_end_count++;
                }
            }
            prev_level = level;

            const QString section_name = get_section_title(section);
            const QString section_path = get_section_path(section);
            out.append(QString("<section title=\"%1\" ref=\"%2.html\">").arg(section_name, section_path));
            section_start_count++;
        }

        // Close all end sections
        for (int i = 0; i < (section_start_count - section_end_count); i++) {
            out.append("</section>");
        }

        out.append("</toc>");

        out.append("<keywords></keywords>");
        
        out.append("<files>");
        for (const QList<QString> &section : section_list) {
            const QString path = get_section_path(section);
            out.append(QString("<file>%1.html</file>").arg(path));
        }
        out.append("</files>");
        out.append("</filterSection>");
        out.append("</QtHelpProject>");

        return out;
    }();

    const QString qhp_file_path = QString("%1/admc.qhp").arg(dest_dir);
    QFile qhp_file(qhp_file_path);

    const bool open_qhp_success = qhp_file.open(QIODevice::WriteOnly);

    if (!open_qhp_success) {
        qCritical() << "Failed to open qhp file:" << qhp_file_path;
        
        return 1;
    }

    qhp_file.write(qhp_file_contents.join("\n").toUtf8());

    qhp_file.close();

    return 0;
}

QString get_section_title(const QList<QString> &section) {
    QString out = section[0];

    out.remove("====");
    out.remove("===");
    out.remove("==");

    while (out.left(1) == " ") {
        out.remove(0, 1);
    }

    while (out.right(1) == " ") {
        out.remove(out.size() - 1, 1);
    }

    return out;
}

QString get_section_path(const QList<QString> &section) {
    QString out = get_section_title(section);

    // NOTE: section titles in wiki can contain forward
    // slashes. Since we're converting them to paths, we
    // need to replace with "|', because forward slashes are
    // invalid chars for paths.
    out.replace("/", "|");

    return out;
}

LineType get_line_type(const QList<QString> &line_list) {
    const QString line = line_list.first();

    if (line.startsWith(OL_string)) {
        return LineType_OL;
    } else if (line.startsWith(UL_string)) {
        return LineType_UL;
    } else if (line.startsWith(UL_2_string)) {
        return LineType_UL_2;
    } else {
        return LineType_Normal;
    }
}

// NOTE: "generate" f-ns operate on the same non-const list
// and pop front elements from the list as they go through
// it
QList<QString> generate_body(const QList<QString> &line_list_const) {
    QList<QString> out;
    
    QList<QString> line_list = line_list_const;

    // Remove all file links
    for (const QString &line : line_list) {
        const bool line_has_file = line.contains("[[");
        const bool line_has_category = line.contains("{{Category");

        if (line_has_file || line_has_category) {
            line_list.removeAll(line);
        }
    }

    // Skip first line (section name)
    line_list.takeFirst();

    while (!line_list.isEmpty()) {
        const LineType type = get_line_type(line_list);

        if (type == LineType_OL) {
            const QList<QString> ol = generate_OL(line_list);
            out.append(ol);
        } else if (type == LineType_Normal) {
            const QString line = line_list.takeFirst();
            out.append(line);
            out.append(BR_string);
        } else {
            qCritical() << "Encountered unknown line structure, maybe it's the \"UL outside OL\" case.";

            break;
        }
    }

    for (int i = 0; i < out.size(); i++) {
        QString line = out[i];
        
        // Change quotations into bold text
        line.replace("«", "<b>");
        line.replace("»", "</b>");

        // Remove wiki-style list markers
        auto remove_first = [&](const QString &string) {
            if (line.startsWith(string)) {
                line.remove(0, string.size());
            }
        };
        remove_first(OL_string);
        remove_first(UL_string);
        remove_first(UL_2_string);

        // Change notes into block quotes
        line.replace("{{Note|", "<blockquote>");
        line.replace("}}", "</blockquote>");

        out[i] = line;
    }

    return out;
}

QList<QString> generate_OL(QList<QString> &line_list) {
    QList<QString> out;

    out.append(OL_start);

    while (!line_list.isEmpty()) {
        const LineType type = get_line_type(line_list);
        if (type == LineType_Normal) {
            break;
        }

        const QString line = line_list.takeFirst();

        out.append(LI_start);
        out.append(line);

        const LineType next_type = get_line_type(line_list);
        if (next_type == LineType_UL) {
            const QList<QString> ul = generate_UL(line_list);
            out.append(ul);
        }

        out.append(LI_end);
    }

    out.append(OL_end);

    return out;
}

QList<QString> generate_UL(QList<QString> &line_list) {
    QList<QString> out;

    out.append(UL_start);

    while (!line_list.isEmpty()) {
        const LineType type = get_line_type(line_list);

        if (type == LineType_OL || type == LineType_Normal) {
            break;
        }

        const QString line = line_list.takeFirst();

        out.append(LI_start);
        out.append(line);

        const LineType next_type = get_line_type(line_list);
        if (next_type == LineType_UL_2) {
            const QList<QString> ul_2 = generate_UL_2(line_list);
            out.append(ul_2);
        }

        out.append(LI_end);
    }

    out.append(UL_end);

    return out;
}


QList<QString> generate_UL_2(QList<QString> &line_list) {
    QList<QString> out;

    out.append(UL_start);

    while (!line_list.isEmpty()) {
        const LineType type = get_line_type(line_list);

        if (type != LineType_UL_2) {
            break;
        }

        const QString line = line_list.takeFirst();

        out.append(LI_start);
        out.append(line);
        out.append(LI_end);
    }

    out.append(UL_end);

    return out;
}
