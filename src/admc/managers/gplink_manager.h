#ifndef GPLINKMANAGER_H
#define GPLINKMANAGER_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QMutex>

class GPLinkManager final : public QObject {
    Q_OBJECT

public:
    explicit GPLinkManager(QObject *parent = nullptr);

    void update();
    void set_gplink(const QString &ou_dn, const QString &gplink_str);
    QString ou_gplink(const QString &ou_dn) const;
    bool update_failed();
    const QHash<QString, QString>& gplinks_map() const;
    QStringList linked_ou_list(const QString &policy_dn) const;

private:
    // OU DN - key, GPLink string - value. DN keys can contain domain dn.
    QHash<QString, QString> ou_links;
    mutable QMutex mutex;
    bool is_updated;
    bool failed_to_update = false;
};

#endif // GPLINKMANAGER_H
