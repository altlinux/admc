%define _unpackaged_files_terminate_build 1

Name: admc
Version: 0.11.0
Release: alt1

Summary: AD editor
License: GPLv3+
Group: Other
Url: https://github.com/altlinuxteam/admc

BuildRequires(pre): cmake
BuildRequires(pre): rpm-macros-cmake
BuildRequires(pre): gcc-c++
BuildRequires(pre): qt5-base-devel
BuildRequires(pre): qt5-tools-devel
BuildRequires(pre): catch2-devel
BuildRequires(pre): cmake-modules

BuildRequires: samba-devel
BuildRequires: libldap-devel
BuildRequires: libsasl2-devel
BuildRequires: libsmbclient-devel
BuildRequires: qt5-base-common
BuildRequires: doxygen
BuildRequires: libuuid-devel
BuildRequires: glib2-devel
BuildRequires: libpcre-devel
BuildRequires: libkrb5-devel

Requires: libldap
Requires: libsasl2
Requires: libsasl2-plugin-gssapi
Requires: libsmbclient
Requires: libuuid
Requires: qt5-base-common
Requires: glib2
Requires: libkrb5

Source0: %name-%version.tar

%package test
Summary: Tests for ADMC
Group: Other

%description
AD editor

%description test
Tests for ADMC

%prep
%setup -q

%build
%cmake -DCMAKE_INSTALL_LIBDIR=%_libdir
%cmake_build

%install
%cmake_install

%files
%doc README.md
%doc CHANGELOG.txt
%doc CHANGELOG_ru.txt
%_bindir/admc
%_libdir/libadldap.so
%_man1dir/admc*
%_datadir/applications/admc.desktop
%_iconsdir/hicolor/scalable/apps/admc.svg

%files test
%_libdir/libadmctest.so
%_bindir/admc_test_ad_interface
%_bindir/admc_test_ad_security
%_bindir/admc_test_unlock_edit
%_bindir/admc_test_upn_edit
%_bindir/admc_test_string_edit
%_bindir/admc_test_string_large_edit
%_bindir/admc_test_country_edit
%_bindir/admc_test_gplink
%_bindir/admc_test_select_base_widget
%_bindir/admc_test_filter_widget
%_bindir/admc_test_attributes_tab
%_bindir/admc_test_members_tab
%_bindir/admc_test_member_of_tab
%_bindir/admc_test_select_object_dialog
%_bindir/admc_test_logon_hours_dialog
%_bindir/admc_test_logon_computers_edit
%_bindir/admc_test_expiry_edit
%_bindir/admc_test_password_edit
%_bindir/admc_test_group_scope_edit
%_bindir/admc_test_group_type_edit
%_bindir/admc_test_datetime_edit
%_bindir/admc_test_manager_edit
%_bindir/admc_test_delegation_edit
%_bindir/admc_test_string_other_edit
%_bindir/admc_test_account_option_edit
%_bindir/admc_test_gpoptions_edit
%_bindir/admc_test_protect_deletion_edit
%_bindir/admc_test_octet_attribute_dialog
%_bindir/admc_test_bool_attribute_dialog
%_bindir/admc_test_datetime_attribute_dialog
%_bindir/admc_test_string_attribute_dialog
%_bindir/admc_test_number_attribute_dialog
%_bindir/admc_test_list_attribute_dialog
%_bindir/admc_test_edit_query_item_widget
%_bindir/admc_test_policy_results_widget
%_bindir/admc_test_policy_ou_results_widget
%_bindir/admc_test_find_object_dialog
%_bindir/admc_test_rename_object_dialog
%_bindir/admc_test_create_object_dialog
%_bindir/admc_test_select_classes_widget
%_bindir/admc_test_sam_name_edit
%_bindir/admc_test_dn_edit

# NOTE: remove this changelog entry when merging into sisyphus branch. This is an auto-generated changelog entry for the upstream branch. For release builds, you should add a hand-written changelog entry.
%changelog
* %(LC_TIME=C date "+%%a %%b %%d %%Y") %{?package_signer:%package_signer}%{!?package_signer:%packager} %version-%release
- %version-%release
