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

#ifndef AD_ENUMS_H
#define AD_ENUMS_H

enum AccountOption {
    AccountOption_Disabled,
    AccountOption_PasswordExpired,
    AccountOption_DontExpirePassword,
    AccountOption_UseDesKey,
    AccountOption_SmartcardRequired,
    AccountOption_CantDelegate,
    AccountOption_DontRequirePreauth,
    AccountOption_COUNT
};

enum GroupScope {
    GroupScope_Global,
    GroupScope_DomainLocal,
    GroupScope_Universal,
    GroupScope_COUNT
};

enum GroupType {
    GroupType_Security,
    GroupType_Distribution,
    GroupType_COUNT
};

enum SystemFlagsBit {
    SystemFlagsBit_CannotMove = 0x04000000,
    SystemFlagsBit_CannotRename = 0x08000000,
    SystemFlagsBit_CannotDelete = 0x80000000
};

#endif /* AD_ENUMS_H */
