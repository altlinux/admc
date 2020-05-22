
#ifndef ENTRY_CONTEXT_MENU_H
#define ENTRY_CONTEXT_MENU_H

#include <QObject>
#include <QMenu>

class QWidget;
class QPoint;
class QString;
class QTreeView;

class EntryContextMenu : public QObject {
Q_OBJECT

public:
    EntryContextMenu();
    void connect_view(const QTreeView &view);

public slots:

signals:
    void attributes_clicked(const QString &dn);
    void delete_clicked(const QString &dn);

private:
    QMenu menu;
    QString target_dn;

};

#endif /* ENTRY_CONTEXT_MENU_H */
