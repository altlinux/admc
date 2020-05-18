
#ifndef ENTRY_CONTEXT_MENU_H
#define ENTRY_CONTEXT_MENU_H

#include <QMenu>

class QWidget;
class QPoint;
class QString;
class QTreeView;

class EntryContextMenu : public QMenu {
Q_OBJECT

public:
    explicit EntryContextMenu(QWidget *parent);
    void connect_view(const QTreeView &view);

public slots:

signals:
    void attributes_clicked(const QString &dn);
    void delete_clicked(const QString &dn);

private:
    QString target_dn;

    using QMenu::popup;

};

#endif /* ENTRY_CONTEXT_MENU_H */
