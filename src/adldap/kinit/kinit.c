/* -*- mode: c; c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* clients/kinit/kinit.c - Initialize a credential cache */
/*
 * Copyright 1990, 2008 by the Massachusetts Institute of Technology.
 * All Rights Reserved.
 *
 * Export of this software from the United States of America may
 *   require a specific license from the United States Government.
 *   It is the responsibility of any person or organization contemplating
 *   export to obtain such a license before exporting.
 *
 * WITHIN THAT CONSTRAINT, permission to use, copy, modify, and
 * distribute this software and its documentation for any purpose and
 * without fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright notice and
 * this permission notice appear in supporting documentation, and that
 * the name of M.I.T. not be used in advertising or publicity pertaining
 * to distribution of the software without specific, written prior
 * permission.  Furthermore if you modify this software you must label
 * your software as modified software and not distribute it in such a
 * fashion that it might be confused with the original M.I.T. software.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is" without express
 * or implied warranty.
 */

#include <krb5.h>
#include <stdio.h>

#ifdef HAVE_PWD_H
#include <pwd.h>
static char *
get_name_from_os()
{
    struct passwd *pw;

    pw = getpwuid(getuid());
    return (pw != NULL) ? pw->pw_name : NULL;
}
#else /* HAVE_PWD_H */
static char *
get_name_from_os()
{
    return NULL;
}
#endif

typedef enum { INIT_PW, INIT_KT, RENEW, VALIDATE } action_type;

struct k_opts
{
    /* In seconds */
    krb5_deltat starttime;
    krb5_deltat lifetime;
    krb5_deltat rlife;

    int forwardable;
    int proxiable;
    int request_pac;
    int anonymous;
    int addresses;

    int not_forwardable;
    int not_proxiable;
    int not_request_pac;
    int no_addresses;

    int verbose;

    char *principal_name;
    char *service_name;
    char *keytab_name;
    char *password;
    char *k5_in_cache_name;
    char *k5_out_cache_name;
    char *armor_ccache;

    action_type action;
    int use_client_keytab;

    int num_pa_opts;
    krb5_gic_opt_pa_data *pa_opts;

    int canonicalize;
    int enterprise;
};

struct k5_data
{
    krb5_context ctx;
    krb5_ccache in_cc, out_cc;
    krb5_principal me;
    char *name;
    krb5_boolean switch_to_cache;
};

static int
k5_begin(struct k_opts *opts, struct k5_data *k5, char *error_buf, int *shift, int buffSize)
{
    krb5_error_code ret;
    int success = 0;
    int flags = opts->enterprise ? KRB5_PRINCIPAL_PARSE_ENTERPRISE : 0;
    krb5_ccache defcache = NULL;
    krb5_principal defcache_princ = NULL, princ;
    const char *deftype = NULL;
    char *defrealm, *name;

    ret = krb5_init_context(&k5->ctx);
    if (ret) {
        *shift += snprintf(error_buf, buffSize - *shift, "Error while initializing Kerberos 5 library\n");
        return 0;
    }


    if (opts->k5_out_cache_name) {
        ret = krb5_cc_resolve(k5->ctx, opts->k5_out_cache_name, &k5->out_cc);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while resolving ccache %s\n", opts->k5_out_cache_name);
            goto cleanup;
        }
        if (opts->verbose) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while using specified cache: %s\n", opts->k5_out_cache_name);
        }
    } else {
        /* Resolve the default ccache and get its type and default principal
         * (if it is initialized). */
        ret = krb5_cc_default(k5->ctx, &defcache);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while getting default ccache\n");
            goto cleanup;
        }
        deftype = krb5_cc_get_type(k5->ctx, defcache);
        if (krb5_cc_get_principal(k5->ctx, defcache, &defcache_princ) != 0)
            defcache_princ = NULL;
    }

    /* Choose a client principal name. */
    if (opts->principal_name != NULL) {
        /* Use the specified principal name. */
        ret = krb5_parse_name_flags(k5->ctx, opts->principal_name, flags,
                                    &k5->me);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error when parsing name %s\n", opts->principal_name);
            goto cleanup;
        }
    } else if (opts->anonymous) {
        /* Use the anonymous principal for the local realm. */
        ret = krb5_get_default_realm(k5->ctx, &defrealm);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while getting default realm\n");
            goto cleanup;
        }
        ret = krb5_build_principal_ext(k5->ctx, &k5->me,
                                       strlen(defrealm), defrealm,
                                       strlen(KRB5_WELLKNOWN_NAMESTR),
                                       KRB5_WELLKNOWN_NAMESTR,
                                       strlen(KRB5_ANONYMOUS_PRINCSTR),
                                       KRB5_ANONYMOUS_PRINCSTR, 0);
        krb5_free_default_realm(k5->ctx, defrealm);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while building principal\n");
            goto cleanup;
        }
    } else if (opts->action == INIT_KT) {
        /* Use the default host/service name. */
        ret = krb5_sname_to_principal(k5->ctx, NULL, NULL, KRB5_NT_SRV_HST,
                                      &k5->me);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error when creating default server principal name\n");
            goto cleanup;
        }
    } else if (k5->out_cc != NULL) {
        /* If the output ccache is initialized, use its principal. */
        if (krb5_cc_get_principal(k5->ctx, k5->out_cc, &princ) == 0)
            k5->me = princ;
    } else if (defcache_princ != NULL) {
        /* Use the default cache's principal, and use the default cache as the
         * output cache. */
        k5->out_cc = defcache;
        defcache = NULL;
        k5->me = defcache_princ;
        defcache_princ = NULL;
    }

    /* If we still haven't chosen, use the local username. */
    if (k5->me == NULL) {
        name = get_name_from_os();
        if (name == NULL) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error: Unable to identify user\n");
            goto cleanup;
        }
        ret = krb5_parse_name_flags(k5->ctx, name, flags, &k5->me);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error when parsing name %s\n", name);
            goto cleanup;
        }
    }

    if (k5->out_cc == NULL && krb5_cc_support_switch(k5->ctx, deftype)) {
        /* Use an existing cache for the client principal if we can. */
        ret = krb5_cc_cache_match(k5->ctx, k5->me, &k5->out_cc);
        if (ret && ret != KRB5_CC_NOTFOUND) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while searching for ccache for %s\n", opts->principal_name);
            goto cleanup;
        }
        if (!ret) {
            if (opts->verbose) {
                *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error with using existing cache: %s\n", krb5_cc_get_name(k5->ctx, k5->out_cc));
            }
            k5->switch_to_cache = 1;
        } else if (defcache_princ != NULL) {
            /* Create a new cache to avoid overwriting the initialized default
             * cache. */
            ret = krb5_cc_new_unique(k5->ctx, deftype, NULL, &k5->out_cc);
            if (ret) {
                *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while generating new ccache\n");
                goto cleanup;
            }
            if (opts->verbose) {
                *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error with using new cache: %s\n", krb5_cc_get_name(k5->ctx, k5->out_cc));
            }
            k5->switch_to_cache = 1;
        }
    }

    /* Use the default cache if we haven't picked one yet. */
    if (k5->out_cc == NULL) {
        k5->out_cc = defcache;
        defcache = NULL;
        if (opts->verbose) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error using default cache: %s\n", krb5_cc_get_name(k5->ctx, k5->out_cc));
        }
    }

    if (opts->k5_in_cache_name) {
        ret = krb5_cc_resolve(k5->ctx, opts->k5_in_cache_name, &k5->in_cc);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while resolving ccache %s\n", opts->k5_in_cache_name);
            goto cleanup;
        }
        if (opts->verbose) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error with using specified input cache: %s\n", opts->k5_in_cache_name);
        }
    }

    ret = krb5_unparse_name(k5->ctx, k5->me, &k5->name);
    if (ret) {
        *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error when unparsing name\n");
        goto cleanup;
    }
    if (opts->verbose)
    {
        *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error when using principal: %s\n", k5->name);
    }

    opts->principal_name = k5->name;

    success = 1;

cleanup:
    if (defcache != NULL)
        krb5_cc_close(k5->ctx, defcache);
    krb5_free_principal(k5->ctx, defcache_princ);
    return success;
}

static void
k5_end(struct k5_data *k5)
{
    krb5_free_unparsed_name(k5->ctx, k5->name);
    krb5_free_principal(k5->ctx, k5->me);
    if (k5->in_cc != NULL)
        krb5_cc_close(k5->ctx, k5->in_cc);
    if (k5->out_cc != NULL)
        krb5_cc_close(k5->ctx, k5->out_cc);
    krb5_free_context(k5->ctx);
    memset(k5, 0, sizeof(*k5));
}

static int
k5_kinit(struct k_opts *opts, struct k5_data *k5, char* error_buf, int *shift, int buffSize)
{
    int notix = 1;
    krb5_keytab keytab = 0;
    krb5_creds my_creds;
    krb5_error_code ret;
    krb5_get_init_creds_opt *options = NULL;
    krb5_boolean pwprompt = TRUE;
    krb5_principal cprinc;
    krb5_ccache mcc = NULL;
    int i;

    memset(&my_creds, 0, sizeof(my_creds));

    ret = krb5_get_init_creds_opt_alloc(k5->ctx, &options);
    if (ret)
        goto cleanup;

    if (opts->action == INIT_KT && opts->keytab_name != NULL) {
        ret = krb5_kt_resolve(k5->ctx, opts->keytab_name, &keytab);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error when resolving keytab %s\n", opts->keytab_name);
            goto cleanup;
        }
    } else if (opts->action == INIT_KT && opts->use_client_keytab) {
        ret = krb5_kt_client_default(k5->ctx, &keytab);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error when resolving default client keytab\n");
            goto cleanup;
        }
    }

    for (i = 0; i < opts->num_pa_opts; i++) {
        ret = krb5_get_init_creds_opt_set_pa(k5->ctx, options,
                                             opts->pa_opts[i].attr,
                                             opts->pa_opts[i].value);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while setting '%s'='%s'\n", opts->pa_opts[i].attr, opts->pa_opts[i].value);
            goto cleanup;
        }
        if (opts->verbose) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Erorr with PA Option %s\n", opts->pa_opts[i].value);
        }
    }
    if (k5->in_cc) {
        ret = krb5_get_init_creds_opt_set_in_ccache(k5->ctx, options,
                                                    k5->in_cc);
        if (ret)
            goto cleanup;
    }
    ret = krb5_get_init_creds_opt_set_out_ccache(k5->ctx, options, k5->out_cc);
    if (ret)
        goto cleanup;

    switch (opts->action) {
    case INIT_PW:
        ret = krb5_get_init_creds_password(k5->ctx, &my_creds, k5->me, opts->password,
                                           0, &pwprompt,
                                           opts->starttime, opts->service_name,
                                           options);
        break;
    case INIT_KT:
        ret = krb5_get_init_creds_keytab(k5->ctx, &my_creds, k5->me, keytab,
                                         opts->starttime, opts->service_name,
                                         options);
        break;
    case VALIDATE:
        ret = krb5_get_validated_creds(k5->ctx, &my_creds, k5->me, k5->out_cc,
                                       opts->service_name);
        break;
    case RENEW:
        ret = krb5_get_renewed_creds(k5->ctx, &my_creds, k5->me, k5->out_cc,
                                     opts->service_name);
        break;
    }

    if (ret) {
        /* If reply decryption failed, or if pre-authentication failed and we
         * were prompted for a password, assume the password was wrong. */
        if (ret == KRB5KRB_AP_ERR_BAD_INTEGRITY ||
            (pwprompt && ret == KRB5KDC_ERR_PREAUTH_FAILED)) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Password incorrect\n");
        }
        goto cleanup;
    }

    if (opts->action != INIT_PW && opts->action != INIT_KT) {
        cprinc = opts->canonicalize ? my_creds.client : k5->me;
        ret = krb5_cc_new_unique(k5->ctx, "MEMORY", NULL, &mcc);
        if (!ret)
            ret = krb5_cc_initialize(k5->ctx, mcc, cprinc);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error when creating temporary cache\n");
            goto cleanup;
        }
        if (opts->verbose)
        {
            *shift += snprintf(error_buf, buffSize - *shift, "Error with initialized cache\n");
        }
        ret = krb5_cc_move(k5->ctx, mcc, k5->out_cc);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while saving to cache %s\n", opts->k5_out_cache_name ? opts->k5_out_cache_name : "");
            goto cleanup;
        }
        mcc = NULL;
    }
    notix = 0;
    if (k5->switch_to_cache) {
        ret = krb5_cc_switch(k5->ctx, k5->out_cc);
        if (ret) {
            *shift += snprintf(error_buf + *shift, buffSize - *shift, "Error while switching to new ccache\n");
            goto cleanup;
        }
    }

cleanup:
    if (mcc != NULL)
        krb5_cc_destroy(k5->ctx, mcc);
    if (options)
        krb5_get_init_creds_opt_free(k5->ctx, options);
    if (my_creds.client == k5->me)
        my_creds.client = 0;
    if (opts->pa_opts) {
        free(opts->pa_opts);
        opts->pa_opts = NULL;
        opts->num_pa_opts = 0;
    }
    krb5_free_cred_contents(k5->ctx, &my_creds);
    if (keytab != NULL)
        krb5_kt_close(k5->ctx, keytab);
    return notix ? 0 : 1;
}

int kinit(char *principal, char *password, char *error_buf, int buffSize)
{
    struct k_opts opts;
    struct k5_data k5;
    int authed_k5 = 0;

    memset(&opts, 0, sizeof(opts));
    opts.action = INIT_PW;

    memset(&k5, 0, sizeof(k5));

    opts.principal_name = principal;
    opts.password = password;

    opts.action = INIT_PW;

    int shift = 0;

    if (k5_begin(&opts, &k5, error_buf, &shift, buffSize))
    {
        authed_k5 = k5_kinit(&opts, &k5, error_buf, &shift, buffSize);
    }

    k5_end(&k5);

    if (!authed_k5)
    {
        snprintf(error_buf + shift, buffSize - shift, "Authentication to Kerberos v5 didn't succeed\n");

        return 0;
    }

    return 1;
}
