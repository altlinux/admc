/*
 * ADMC - AD Management Center
 *
 * Copyright (C) 2020-2024 BaseALT Ltd.
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

#include "smb_context.h"

#include <libsmbclient.h>

SMBContext::SMBContext() : smb_ctx_ptr(createContext(), freeContext) {
    if (is_valid()) {
        smbc_set_context(smb_ctx_ptr.get());
    }
}

bool SMBContext::is_valid() const {
    return bool(smb_ctx_ptr);
}

int SMBContext::smbcGetxattr(const char *fname, const char *name, const void *value, size_t size) {
    return smbc_getFunctionGetxattr(smb_ctx_ptr.get())(smb_ctx_ptr.get(), fname, name, value, size);
}

SMBCCTX *SMBContext::createContext() {
    SMBCCTX* newContext = smbc_new_context();

    if (newContext) {
        // smbc_setDebug(newContext, SMB_DEBUG_LEVEL);
        smbc_setOptionUseKerberos(newContext, true);
        smbc_setOptionFallbackAfterKerberos(newContext, true);

        if (smbc_init_context(newContext) == nullptr) {
            smbc_free_context(newContext, SMB_FREE_EVEN_IF_BUSY);
            newContext = nullptr;
        }
    }

    return newContext;
}

void SMBContext::freeContext(SMBCCTX *context) {
    if (context) {
        smbc_free_context(context, SMB_FREE_EVEN_IF_BUSY);
    }
}
