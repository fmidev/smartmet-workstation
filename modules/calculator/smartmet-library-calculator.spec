%define DIRNAME calculator
%define LIBNAME smartmet-%{DIRNAME}
%define SPECNAME smartmet-library-%{DIRNAME}
%define DEVELNAME %{SPECNAME}-devel
Summary: Querydata calculator
Name: %{SPECNAME}
Version: 17.1.27
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-library-calculator
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: boost-devel
BuildRequires: smartmet-library-newbase-devel >= 17.1.10
Requires: smartmet-library-newbase >= 17.1.10
Provides: %{LIBNAME}

%description
Querydata calculator library

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{SPECNAME}
 
%build
make %{_smp_mflags}

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,0775)
%{_libdir}/lib%{LIBNAME}.so

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%package -n %{DEVELNAME}
Summary: FMI %{DIRNAME} development files
Provides: %{DEVELNAME}

%description -n %{DEVELNAME}
%{DIRNAME} development files

%files -n %{DEVELNAME}
%defattr(0664,root,root,-)
%{_includedir}/smartmet/%{DIRNAME}/*.h

%changelog
* Fri Jan 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.27-1.fmi
- Recompiled due to NFmiQueryData object size change

* Thu Jan 12 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.12-1.fmi
- First version extracted from the full text generator library
