%define _unpackaged_files_terminate_build 1

Name: adtool
Version: 0.1.0
Release: alt1

Summary: AD editor
License: GPLv2+
Group: Other
Url: https://github.com/altlinuxteam/adtool
BuildArch: x86_64

BuildRequires(pre): cmake
BuildRequires(pre): rpm-macros-cmake
BuildRequires(pre): gcc-c++
BuildRequires(pre): qt5-base-devel

BuildRequires: openldap-devel
BuildRequires: libsasl2-devel
BuildRequires: qt5-base-common
BuildRequires: doxygen

Requires: openldap
Requires: libsasl2
Requires: qt5-base-common

Source0: %name-%version.tar

%description
AD editor

%prep
%setup -q

%build
%cmake
%cmake_build VERBOSE=1

%install
cd BUILD
%makeinstall_std

%files
%doc README.md
%_bindir/adtool

%changelog
* Thu May 21 2020 Igor Chudov <nir@altlinux.org> 0.1.0-alt1
- Initial build

