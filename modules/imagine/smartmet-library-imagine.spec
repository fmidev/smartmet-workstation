%define DIRNAME imagine
%define LIBNAME smartmet-%{DIRNAME}
%define SPECNAME smartmet-library-%{DIRNAME}
%define DEVELNAME %{SPECNAME}-devel
Summary: imagine library
Name: %{SPECNAME}
Version: 17.8.28
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-library-imagine
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
BuildRequires: gcc-c++
BuildRequires: make
BuildRequires: boost-devel
BuildRequires: freetype-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: smartmet-library-newbase-devel >= 17.2.10
BuildRequires: zlib-devel
BuildRequires: cairomm-devel
Requires: smartmet-library-newbase >= 17.2.10
Requires: boost-filesystem
Requires: boost-regex
Requires: boost-thread
Requires: cairomm
Requires: freetype
Requires: libjpeg
Requires: libpng
Requires: zlib
Provides: %{LIBNAME}
Obsoletes: libsmartmet-imagine < 17.1.4
Obsoletes: libsmartmet-imagine-debuginfo < 17.1.4

%description
FMI imagine rendering library

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
Obsoletes: libsmartmet-imagine-devel < 17.1.4

%description -n %{DEVELNAME}
FMI %{DIRNAME} development files

%files -n %{DEVELNAME}
%defattr(0664,root,root,-)
%{_includedir}/smartmet/%{DIRNAME}/*.h

%changelog
* Mon Aug 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.8.28-1.fmi
- Upgrade to boost 1.65

* Tue Mar 14 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.14-1.fmi
- Made SmartMet include paths explicit

* Fri Feb 10 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.10-1.fmi
- newbase API changed forced rebuild

* Wed Jan  4 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.4-1.fmi
- Switched to using FMI open source naming conventions

* Thu Oct 27 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.10.27-1.fmi
- Added more tolerance to Pacific view guessing

* Sun Jan 17 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.1.17-1.fmi
- newbase API changed

* Wed Apr 15 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.15-1.fmi
- newbase API changed

* Thu Apr  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.9-1.fmi
- newbase API changed

* Mon Mar 30 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.3.30-1.fmi
- Switched to dynamic linkage

* Fri Feb  6 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.6-1.fmi
- Recompiled with the latest newbase

* Thu Sep 25 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.9.25-1.fmi
- New improved color reduction algorithm

* Wed Nov 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.27-1.fmi
- Recompiled due to newbase API changes

* Thu Oct 17 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.10.17-1.fmi
- Recompiled due to newbase API changes

* Fri Sep 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.27-1.fmi
- Added NFmiEsriTools for filtering shapes
- Fixed shapefilter and shapepoints segmentation faults

* Thu Sep  5 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.5-1.fmi
- Compiled with the latest newbase

* Fri Aug  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.8.2-4.fmi
- Permit very long line segments at the poles, they are really of length zero

* Fri Aug  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.8.2-3.fmi
- Fixed a problem with some shapes the have longitudes slightly beyond 180

* Fri Aug  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.8.2-2.fmi
- Bug fix to forcing Pacific paths into Atlantic mode

* Fri Aug  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.8.2-1.fmi
- Added code to enable Pacific views of data
- Added contouring of automatically detected global data with wrap-around at the edges

* Wed Jul  3 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.7.3-1.fmi
- Update to boost 1.54

* Thu Mar 21 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.3.21-1.fmi
- Replaced auto_ptr with shared_ptr

* Wed Nov  7 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.11.7-1.fmi
- Upgrade to boost 1.52

* Fri Jul  6 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.6-1.el6.fmi
- Fixed all issues reported by cppcheck

* Thu Jul  5 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.5-1.el6.fmi
- Migration to boost 1.50

* Sat Mar 31 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.3.31-1.el5.fmi
- RHEL5 specific release with latest newbase math functions

* Wed Mar 28 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.3.28-1.el6.fmi
- Upgrade to boost 1.49

* Mon Feb 27 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.27-1.el6.fmi
- Upgraded newbase

* Mon Oct 17 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.10.17-1.el5.fmi
- Upgrade to newbase 11.10.17

* Wed Jul 20 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.7.20-1.el5.fmi
- Upgrade to boost 1.47

* Tue May 31 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.5.31-1.el5.fmi
- Major newbase upgrade

* Fri May 20 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.5.20-1.el6.fmi
- Upgrade to RHEL6

* Thu Mar 24 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.3.24-1.el5.fmi
- Upgrade to boost 1.46

* Thu Feb  3 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.2.3-1.el5.fmi
- Upgraded to latest newbase

* Tue Sep 14 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.9.14-1.el5.fmi
- Upgraded to boost 1.44 and newbase 10.9.14-1

* Wed Jul  7 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.7.5-1.el5.fmi
- Upgraded to newbase 10.7.5-1

* Fri Jan 15 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.1.15-1.el5.fmi
- Upgraded to boost 1.41

* Tue Jan  5 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.1.5-1.el5.fmi
- Fixed TrueType font rendering

* Tue Jul 14 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.4.17-1.el5.fmi
- Recompiled with boost 1.39

* Mon Apr  6 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.4.6-1.el5.fmi
- Recompiled due to newbase and WIN32 changes

* Wed Mar 18 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.3.18-1.el5.fmi
- Recompiled due to newbase API changes

* Wed Nov 12 2008 pkeranen <pekka.keranen@fmi.fi> - 8.11.12-1.el5.fmi
- More improved shapefile support

* Mon Nov 10 2008 pkeranen <pekka.keranen@fmi.fi> - 8.11.10-1.el5.fmi
- Improved shapefile support

* Mon Sep 29 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.29-1.el5.fmi
- Newbase header change forced recompile

* Mon Sep 22 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.22-3.el5.fmi
- Restored jpeg support to be on by default

* Mon Sep 22 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.22-2.el5.fmi
- Fixed typedef when Cairo is not in use

* Mon Sep 22 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.22-1.el5.fmi
- Updates from Asko and compiled with static boost 1.36

* Mon Sep 15 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.15-1.el5.fmi
- Compiled with boost 1.36 and latest newbase

* Tue Jul 15 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.7.15-1.el5.fmi
- Linked with newest newbase

* Mon May 19 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.5.19-1.el5.fmi
- Added partial support for line width in stroking paths

* Tue Apr 15 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.4.15-1.el5.fmi
- Linked with newbase 8.4.15-2 with Asko's mmap technology

* Tue Mar 11 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.3.11-1.el5.fmi
- Linked with newbase 8.3.11-1 with wind interpolation fixes

* Wed Jan 30 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.1.30-1.el5.fmi
- Linked with newbase 8.1.25-1 for getting new parameter names

* Thu Dec 27 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.7-1.el5.fmi
- Linked with newbase 1-0.7 with wind direction bugfixes

* Wed Dec 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.6-1.el5.fmi
- API fixes

* Fri Dec 14 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.5-1.el5.fmi
- Added new GetType method to NFmiEsriElement

* Fri Nov 30 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.4-1.el5.fmi
- Linked with newbase 1.0.5-1

* Mon Nov 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.3-1.el5.fmi
- Linked with newbase 1.0.4-1

* Thu Nov 15 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.2-1.el5.fmi
- Linked with newbase 1.0.3-1
- Fixed regression test build system                 

* Mon Sep 24 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-4.el5.fmi
- Fixed "make depend".

* Fri Sep 14 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-3.el5.fmi
- Added "make tag" feature.

* Thu Sep 13 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-2.el5.fmi
- Improved make system.

* Thu Jun  7 2007 tervo <tervo@xodin.weatherproof.fi> - 
- Initial build.

