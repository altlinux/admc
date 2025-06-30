/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2025 BaseALT Ltd.
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

#include "krb5client.h"
#include <krb5.h>
#include <stdexcept>
#include <QCoreApplication>
#include <ctime>
#include <QDebug>
#include <unistd.h>
#include <sys/types.h>
#include <QHash>
#include <QDir>

class Krb5Client::Krb5ClientImpl final {
public:
    krb5_context context;
    QString curr_principal;
    QString sys_principal;
    QHash<QString, Krb5TGTData> principal_tgt_map;
    QHash<QString, krb5_ccache> principal_cache_map;
    bool use_default_cache;
    static const QString ccaches_path;
    static const QString ccache_name_prefix;

    explicit Krb5ClientImpl();
    ~Krb5ClientImpl();

    void kinit(const QString &principal, const QString &password);
    void load_caches();
    void load_cache_data(krb5_ccache ccache, bool is_system);
    Krb5TgtState tgt_state_from_creds(const krb5_creds &creds);
    void update_tgt_state_from_creds(Krb5TGTData &data, const krb5_creds &creds);
    void throw_error(const QString &error, krb5_error_code err_code);
    void cleanup(krb5_ccache ccache, krb5_creds *creds, krb5_principal principal, char *principal_unparsed);
    void cleanup_and_throw(const QString &error, krb5_error_code err_code, krb5_ccache ccache, krb5_creds *creds,
                           krb5_principal principal, char *principal_unparsed);
    QString principal_from_ccache(krb5_ccache ccache);
};

const QString Krb5Client::Krb5ClientImpl::ccaches_path = QString("/tmp/admc_uid") + QString::number(getuid()) + "/ccaches/";
const QString Krb5Client::Krb5ClientImpl::ccache_name_prefix = "krb5cc_";

Krb5Client::Krb5ClientImpl::Krb5ClientImpl() : context(NULL) {
    krb5_error_code res = krb5_init_context(&context);
    QString error;
    if (res) {
        error = QCoreApplication::translate("Krb5Client", "Kerberos initialization failed");
        throw(std::runtime_error(error.toUtf8().data()));
    }

    QDir dir(ccaches_path);
    if (!dir.mkpath(".")) {
        error = QCoreApplication::translate("Krb5Client", "Failed to create caches path");
        throw(std::runtime_error(error.toUtf8().data()));
    }

    dir.cdUp();
    QFile file(dir.absolutePath());
    if (!file.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner)) {
        error = QCoreApplication::translate("Krb5Client", "Failed to set caches path permissions");
        throw(std::runtime_error(error.toUtf8().data()));
    }

    load_caches();
}


Krb5Client::Krb5ClientImpl::~Krb5ClientImpl() {
    krb5_free_context(context);
    for (krb5_ccache ccache : principal_cache_map.values()) {
        krb5_cc_close(context, ccache);
    }
}

void Krb5Client::Krb5ClientImpl::kinit(const QString &principal, const QString &password) {
    krb5_creds creds;
    krb5_error_code res;
    QString error = QCoreApplication::translate("Krb5Client", "Authentication failed");

    const QByteArray principal_bytes = principal.toUtf8();
    const char *principal_name = principal_bytes.constData();
    krb5_principal princ;
    res = krb5_parse_name(context, principal_name, &princ);
    if (res) {
        throw_error(error, res);
    }

    memset(&creds, 0, sizeof(creds));
    const QByteArray passwd_bytes = password.toUtf8();
    const char *passwd = passwd_bytes.constData();

    // Setup renewal times for test
    // krb5_get_init_creds_opt *opts;
    // krb5_get_init_creds_opt_alloc(context, &opts);
    // krb5_get_init_creds_opt_set_tkt_life(opts, 666);
    // krb5_get_init_creds_opt_set_renew_life(opts, 86400);

    res = krb5_get_init_creds_password(context, &creds, princ, passwd, NULL, NULL, 0, NULL, NULL /*opts*/);
    if (res) {
        cleanup_and_throw(error, res, nullptr, &creds, princ, nullptr);
    }

    // krb5_get_init_creds_opt_free(context, opts);

    krb5_ccache ccache;
    const QString cache_name = QString("FILE:") + ccaches_path + ccache_name_prefix +
            principal.section('@', 0, 0);
    const QByteArray cache_name_bytes = cache_name.toUtf8();
    res = krb5_cc_resolve(context, cache_name_bytes.constData(), &ccache);
    if (res) {
        cleanup_and_throw(error, res, ccache, &creds, princ, nullptr);
    }

    res = krb5_cc_initialize(context, ccache, princ);
    if (res) {
        cleanup_and_throw(error, res, ccache, &creds, princ, nullptr);
    }

    res = krb5_cc_store_cred(context, ccache, &creds);
    if (res) {
        cleanup_and_throw(error, res, ccache, &creds, princ, nullptr);
    }

    // Set as the default for subsequent authentication via SASL
    if (setenv("KRB5CCNAME", cache_name_bytes.constData(), 1)) {
        const QString setenv_fail = QCoreApplication::translate("Krb5Client", "Failed to set KRB5CCNAME");
        cleanup_and_throw(setenv_fail, 0, ccache, &creds, princ, nullptr);
    }

    load_cache_data(ccache, true);

    curr_principal = principal;
}

void Krb5Client::Krb5ClientImpl::load_caches() {
    // Load default cache data
    krb5_ccache def_ccache = nullptr;
    krb5_error_code res = krb5_cc_default(context, &def_ccache);
    if (res) {
        qDebug() << "Failed to get default cache";
        krb5_cc_close(context, def_ccache);
        return;
    }
    load_cache_data(def_ccache, true);

    krb5_ccache ccache = nullptr;

    // Load caches from custom custom dir
    QDir dir(ccaches_path);
    for (const QString &ccache_name : dir.entryList({ccache_name_prefix + "*"})) {
        const QByteArray typed_ccname_bytes = QByteArray("FILE:") + ccaches_path.toUtf8() +
                            ccache_name.toUtf8();
        krb5_cc_resolve(context, typed_ccname_bytes.constData(), &ccache);
        QString principal = principal_from_ccache(ccache);

        if (principal.isEmpty() || principal_cache_map.contains(principal)) {
            krb5_cc_close(context, ccache);
            continue;
        }

        load_cache_data(ccache, false);
    }

    curr_principal = sys_principal;
}

void Krb5Client::Krb5ClientImpl::load_cache_data(krb5_ccache ccache, bool is_system) {
    krb5_error_code res;
    krb5_principal principal;
    krb5_creds creds;
    Krb5TGTData tgt_data;

    res = krb5_cc_get_principal(context, ccache, &principal);
    if (res) {
        cleanup(ccache, nullptr, principal, nullptr);
        return;
    }

    char *princ = nullptr;
    res = krb5_unparse_name(context, principal, &princ);
    if (res) {
        cleanup(ccache, nullptr, principal, princ);
        return;
    }
    tgt_data.principal = princ;
    krb5_free_unparsed_name(context, princ);

    memset(&creds, 0, sizeof(creds));
    res = krb5_build_principal(context, &creds.server,
                                   krb5_princ_realm(context, principal)->length,
                                   krb5_princ_realm(context, principal)->data,
                                   "krbtgt",
                                   krb5_princ_realm(context, principal)->data,
                                   NULL);

    if (res) {
        cleanup(ccache, &creds, principal, nullptr);
        return;
    }

    res = krb5_cc_retrieve_cred(context, ccache, 0, &creds, &creds);
    if (res) {
        cleanup(ccache, &creds, principal, nullptr);
        return;
    }

    if (is_system) {
        sys_principal = tgt_data.principal;
    }
    tgt_data.state = tgt_state_from_creds(creds);
    tgt_data.realm = principal->realm.data;
    tgt_data.starts.setSecsSinceEpoch(creds.times.starttime);
    tgt_data.renew_until.setSecsSinceEpoch(creds.times.renew_till);
    tgt_data.expires.setSecsSinceEpoch(creds.times.endtime);

    principal_tgt_map[tgt_data.principal] = tgt_data;
    principal_cache_map[tgt_data.principal] = ccache;

    // ccache is not closed here because it can be used later and will be
    // closed in destructor
    cleanup(nullptr, &creds, principal, nullptr);
}

Krb5TgtState Krb5Client::Krb5ClientImpl::tgt_state_from_creds(const krb5_creds &creds) {
    std::time_t now = time(nullptr);
    if (now > creds.times.renew_till) {
        return Krb5TgtState_Outdated;
    }
    else if (now > creds.times.endtime) {
        return Krb5TgtState_Expired;
    }

    return Krb5TgtState_Active;
}

void Krb5Client::Krb5ClientImpl::update_tgt_state_from_creds(Krb5TGTData &data, const krb5_creds &creds) {
    data.expires.setSecsSinceEpoch(creds.times.endtime);
    data.renew_until.setSecsSinceEpoch(creds.times.renew_till);
    data.state = tgt_state_from_creds(creds);
}

void Krb5Client::Krb5ClientImpl::throw_error(const QString &error, krb5_error_code err_code) {
    QString out_err = err_code ? error + QString(": ") + krb5_get_error_message(context, err_code) :
                                 error;
    throw std::runtime_error(out_err.toUtf8().data());
}

void Krb5Client::Krb5ClientImpl::cleanup(krb5_ccache ccache, krb5_creds *creds, krb5_principal principal, char *principal_unparsed) {
    if (ccache) {
        krb5_cc_close(context, ccache);
        ccache = nullptr;
    }

    if (creds) {
        krb5_free_cred_contents(context, creds);
        creds = nullptr;
    }

    if (principal) {
        krb5_free_principal(context, principal);
        principal = nullptr;
    }

    if (principal_unparsed) {
        krb5_free_unparsed_name(context, principal_unparsed);
        principal_unparsed = nullptr;
    }
}

void Krb5Client::Krb5ClientImpl::cleanup_and_throw(const QString &error, krb5_error_code err_code, krb5_ccache ccache, krb5_creds *creds, krb5_principal principal, char *principal_unparsed) {
    cleanup(ccache, creds, principal, principal_unparsed);
    throw_error(error, err_code);
}

QString Krb5Client::Krb5ClientImpl::principal_from_ccache(krb5_ccache ccache) {
    if (!ccache) {
        return QString();
    }

    krb5_principal principal = nullptr;
    krb5_error_code res = krb5_cc_get_principal(context, ccache, &principal);
    if (res) {
        return QString();
    }

    char *princ = nullptr;
    res = krb5_unparse_name(context, principal, &princ);
    if (res) {
        cleanup(nullptr, nullptr, principal, princ);
        return QString();
    }

    cleanup(nullptr, nullptr, principal, princ);
    return princ;
}

Krb5Client::Krb5Client() : impl(std::unique_ptr<Krb5ClientImpl>(new Krb5ClientImpl)) {

}

Krb5Client::~Krb5Client() {

}

void Krb5Client::authenticate(const QString &principal, const QString &password) {
    impl->kinit(principal, password);
}

void Krb5Client::set_current_principal(const QString &principal) {
    QString error;
    if (!impl->principal_cache_map.value(principal, nullptr)) {
        error = QCoreApplication::translate("Krb5Client", "Principal is not found");
        impl->throw_error(error, 0);
    }

//    std::time_t expire_time = impl->principal_tgt_map[principal].expires.toSecsSinceEpoch();
//    std::time_t now = time(nullptr);
//    const bool is_expired = now > expire_time;
//    if (is_expired) {
//        refresh_tgt(principal);
//    }

    krb5_ccache ccache = impl->principal_cache_map[principal];
    const QByteArray ccache_name = QByteArray(krb5_cc_get_type(impl->context, ccache)) + QByteArray(":") +
                                    QByteArray(krb5_cc_get_name(impl->context, ccache));
    int res = setenv("KRB5CCNAME", ccache_name.constData(), 1);
    if (res) {
        error = QCoreApplication::translate("Krb5Client", "Failed to switch principal");
        impl->throw_error(error, res);
    }

    impl->curr_principal = principal;
}

void Krb5Client::refresh_tgt(const QString &principal) {
    QString error = QCoreApplication::translate("Krb5Client", "Failed to refresh TGT");
    krb5_ccache ccache = impl->principal_cache_map.value(principal, nullptr);
    if (!ccache) {
        impl->throw_error(error, 0);
    }

    krb5_principal princ;
    krb5_error_code res = krb5_cc_get_principal(impl->context, ccache, &princ);
    if (res) {
        impl->throw_error(error, res);
    }

    krb5_creds new_creds;
    memset(&new_creds, 0, sizeof(new_creds));

//    const QByteArray ccache_name = QByteArray(krb5_cc_get_type(impl->context, ccache)) + QByteArray(":") +
//                                    QByteArray(krb5_cc_get_name(impl->context, ccache));
//    setenv("KRB5CCNAME", ccache_name.constData(), 1);
    res = krb5_get_renewed_creds(impl->context, &new_creds, princ, ccache, NULL);
    if (res) {
        krb5_free_principal(impl->context, princ);
        impl->throw_error(error, res);
    }

    res = krb5_cc_store_cred(impl->context, ccache, &new_creds);
    if (res) {
        impl->cleanup_and_throw(error, res, nullptr, &new_creds, princ, nullptr);
    }

    impl->update_tgt_state_from_creds(impl->principal_tgt_map[principal], new_creds);
    impl->cleanup(nullptr, &new_creds, princ, nullptr);
}

Krb5TGTData Krb5Client::tgt_data(const QString &principal) const {
    return impl->principal_tgt_map.value(principal, Krb5TGTData());
}

QString Krb5Client::current_principal() const {
    return impl->curr_principal;
}

QString Krb5Client::system_principal() const {
    return impl->sys_principal;
}

bool Krb5Client::principal_has_cache(const QString &principal) const {
    return impl->principal_cache_map.contains(principal);
}

QStringList Krb5Client::available_principals() const {
    return impl->principal_cache_map.keys();
}

QStringList Krb5Client::active_tgt_principals() const {
    QStringList out;
    for (const QString &principal : available_principals()) {
        Krb5TGTData tgt_data = impl->principal_tgt_map[principal];
        if (tgt_data.state == Krb5TgtState_Active /* || tgt_data.state == Krb5TgtState_Expired*/) {
            out << principal;
        }
    }

    return out;
}
