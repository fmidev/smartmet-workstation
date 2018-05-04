%define DIRNAME smarttools
%define LIBNAME smartmet-%{DIRNAME}
%define SPECNAME smartmet-library-%{DIRNAME}
%define DEVELNAME %{SPECNAME}-devel
Summary: smarttools library
Name: %{SPECNAME}
Version: 17.3.28
Release: 1%{?dist}.fmi
License: MIT
Group: Development/Libraries
URL: https://github.com/fmidev/smartmet-library-smarttools
Source: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(%{__id_u} -n)
BuildRequires: smartmet-library-newbase-devel >= 17.3.28
BuildRequires: boost-devel
Requires: smartmet-library-newbase >= 17.3.28
Provides: %{LIBNAME}
Obsoletes: libsmartmet-smarttools < 17.1.4
Obsoletes: libsmartmet-smarttools-debuginfo < 17.1.4

%description
FMI smarttools library

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
%defattr(0775,root,root,0775)
%{_libdir}/lib%{LIBNAME}.so

%post -p /sbin/ldconfig
%postun -p /sbin/ldconfig

%package -n %{DEVELNAME}
Summary: FMI smarttools development files
Provides: %{DEVELNAME}
Obsoletes: libsmartmet-smarttools-devel < 17.1.4

%description -n %{DEVELNAME}
FMI smarttools development files

%files -n %{DEVELNAME}
%defattr(0664,root,root,0775)
%{_includedir}/smartmet/%{DIRNAME}/*.h


%changelog
* Tue Mar 28 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.28-1.fmi
- Refactored some code into newbase

* Thu Mar  9 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.9-1.fmi
- Final merge of the SmartMet Editor code branch, numerous changes

* Mon Mar  6 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.3.6-1.fmi
- Include newbase library headers with correct notation

* Fri Feb 10 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.10-1.fmi
- Merged SmartMet Editor changes

* Sat Feb  4 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.2.4-1.fmi
- Memory map helper files to enable larger input data for qdscript

* Fri Jan 27 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.27-1.fmi
- Recompiled since NFmiQueryData size changed

* Wed Jan  4 2017 Mika Heiskanen <mika.heiskanen@fmi.fi> - 17.1.4-1.fmi
- Switched to FMI open source naming conventions

* Thu Oct 27 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.10.29-1.fmi
- Enable scripting with multiple files with the same producer but different parameters

* Tue Jun 14 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.6.14-1.fmi
- Recompiled due to newbase API changes

* Sun Jan 17 2016 Mika Heiskanen <mika.heiskanen@fmi.fi> - 16.1.17-1.fmi
- "td" is now an alias for dew point, meteorologists prefer it over dp
- Handle sounding querydata
- Bug fix to ThetaE calculations
- Bug fix to parameter min/max limit handling
- Bug fix to progress bar updates
- Bug fix to moving parameters in lists
- Bug fix to level handling with respect to height as in T_EC_z500
- Bug fix to time indexing of SmartMet editor displays

* Wed Apr 15 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.15-1.fmi
- newbase API changed for LatLonCache queries

* Thu Apr  9 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.4.9-1.fmi
- newbase API changed

* Mon Mar 30 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.3.30-1.fmi
- Switched to dynamic linkage

* Mon Mar 16 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.3.16-1.fmi
- Recompiled due to newbase API changes

* Mon Feb 16 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.16-1.fmi
- Fixes to data validation
- Memory leak fixes
- Added capability to detect changes to configuration file settings
- Added smarttool parameter names qnh and icing

* Fri Feb  6 2015 Mika Heiskanen <mika.heiskanen@fmi.fi> - 15.2.6-1.fmi
- Recompiled with the latest newbase

* Thu Oct 30 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.10.30-1.fmi
- New vertical interpolation methods in newbase

* Mon Oct 13 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.10.13-1.fmi
- Fixes to decoding Columbian TEMP headers

* Fri Aug  1 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.8.1-1.fmi
- Fixes to sounding height calculations
- New code for rendering weather observations

* Wed May 28 2014 Santeri Oksman <santeri.oksman@fmi.fi> - 14.5.28-1.fmi
- Recompiled to get WeatherSymbol parameter

* Thu Apr 10 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.4.10-1.fmi
- Recompiled to gain access to pollen parameters

* Mon Apr  7 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.4.7-1.fmi
- Sounding index calculations are now multi threaded
- API change to sounding index functions to enable using Himan

* Mon Dec  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.12.2-1.fmi
- Station IDs can now be negative

* Wed Nov 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.27-1.fmi
- Recompiled due to gnomonic area changes in newbase

* Mon Nov 25 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.25-1.fmi
- Fixes to SYNOP message parsing
- Improvements to sounding data handling

* Thu Oct 17 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.10.17-1.fmi
- Speed optimizations to qdsoundingindex
- Recompiled due to newbase NFmiQueryInfo changes

* Thu Sep 26 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.26-1.fmi
- Recompiled with the latest newbase

* Mon Sep 23 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.23-1.fmi
- Recompiled due to newbase API changes
- Moved colour tools from newbase to smarttools

* Thu Sep  5 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.5-1.fmi
- Compiled with latest newbase with API changes in NFmiQueryData
- Added checks for empty querydata to avoid segmentation faults

* Wed Aug 21 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.8.21-1.fmi
- Station data now uses comma as a field delimiter

* Fri Aug  2 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.8.2-1.fmi
- Compiled with support for 0...360 views of the world

* Mon Jul 22 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.7.22-1.fmi
- Improved thread safety in random number generation

* Wed Jul  3 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.7.3-1.fmi
- Updated to boost 1.54

* Mon Jun 10 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.6.10-1.fmi
- Added support for tracetory history data

* Thu May 23 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.5.23-1.fmi
- Recompiled due to changes in newbase headers

* Thu Mar 21 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.3.21-1.fmi
- Replaced auto_ptr with shared_ptr

* Wed Mar  6 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.3.6-1.fmi
- Compiled with the latest newbase

* Tue Nov 27 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.11.27-2.el6.fmi
- Merged branch smartmet_5_8_final to get bug fixes by Marko Pietarinen

* Tue Nov 27 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.11.27-1.el6.fmi
- Recompiled since newbase headers changed

* Wed Nov  7 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.11.7-1.el6.fmi
- Upgrade to boost 1.52 

* Mon Oct 15 2012 Roope Tervo <roope.tervo@fmi.fi> - 12.10.2-1.el6.fmi
- Changes made by Mikko Partio

* Tue Oct  2 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.10.2-1.el6.fmi
- Newbase API changes force a recompile

* Fri Jul  6 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.6-1.el6.fmi
- Fixed all issues reported by cppcheck

* Thu Jul  5 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.5-1.el6.fmi
- Migration to boost 1.50

* Sat Mar 31 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.3.31-1.el5.fmi
- Upgraded to boost 12.3.31 in el5

* Wed Mar 28 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.3.28-1.el6.fmi
- Upgraded to boost 1.49

* Fri Mar 16 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.3.16-1.el6.fmi
- newbase headers have changed

* Fri Feb 24 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.2.24-1.el6.fmi
- Single threading removed due to extra maintenance required
- A shitload of changes, fixes and improvements by Marko Pietarinen

* Tue Feb  7 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.2.7-1.el5.fmi
- Newbase headers changed

* Thu Nov 24 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.11.24-1.el5.fmi
- Newbase headers changed

* Thu Nov 17 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.11.17-1.el5.fmi
- Recompiled with -fPIC for q2engine

* Wed Nov 16 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.11.16-1.el5.fmi
- Recompiled due to newbase API changes

* Tue Nov  8 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.11.8-1.el5.fmi
- Recompiled due to newbase API changes

* Fri Oct 28 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.10.28-1.el6.fmi
- Bug fix to handling station numbers

* Thu Oct 27 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.10.27-1.el6.fmi
- New code to handle observations from stations

* Mon Oct 17 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.10.17-3.el5.fmi
- newbase headers changed again

* Mon Oct 17 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.10.17-2.el5.fmi
- Numerous improvements and bug fixes from Marko

* Mon Oct 17 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.10.17-1.el5.fmi
- Upgrade to newbase 11.10.17-1

* Wed Jul 20 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.7.20-1.el5.fmi
- Upgrade to newbase 11.7.20-1

* Mon Jun 20 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.6.20-1.el5.fmi
- Improvements needed for new grib tools

* Mon Jun  6 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.6.6-1.el5.fmi
- Newbase headers changed, rebuild forced

* Tue May 31 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.5.31-1.el6.fmi
- Major merge by Marko Pietarinen

* Fri May 20 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.5.20-1.el6.fmi
- RHEL6 release

* Tue Apr 19 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.4.19-1.el5.fmi
- Fixed all error messages to be in English

* Thu Mar 24 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.3.24-1.el5.fmi
- Several updates plus upgraded newbase

* Mon Jan  3 2011 Mika Heiskanen <mika.heiskanen@fmi.fi> - 11.2.3-1.el5.fmi
- Numerous updates by Marko plus upgrade to latest newbase

* Tue Sep 14 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.9.14-1.el5.fmi
- Upgrade to boost 1.44 and newbase 10.9.14-1

* Mon Jul  5 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.7.5-1.el5.fmi
- Compiled with newbase 10.7.5-1

* Thu May  6 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.5.6-1.el5.fmi
- A large number of changes by Marko

* Fri Jan 15 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.1.15-1.el5.fmi
- Recompiled with latest newbase

* Mon Jan 11 2010 Mika Heiskanen <mika.heiskanen@fmi.fi> - 10.1.11-1.el5.fmi
- Numerous minor updates

* Tue Jul 14 2009 Mika Heiskanen <mika.heiskanen@fmi.fi> - 9.7.14-1.el5.fmi
- Upgrade to boost 1.39 
- Minor changes by Marko Pietarinen

* Wed Apr 22 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.4.22-1.el5.fmi
- Compile single- and multithread versions

* Fri Mar 27 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.3.27-1.el5.fmi
- Updates from Marko Pietarinen

* Mon Jan 19 2009 Marko Pietarinen <marko.pietarinen@fmi.fi> - 9.1.20-1.el5.fmi
- Some bugfixes to newbase

* Mon Jan 19 2009 Marko Pietarinen <marko.pietarinen@fmi.fi> - 9.1.19-1.el5.fmi
- Smallish upgrades by Marko

* Tue Sep 30 2008 westerba <antti.westerberg@fmi.fi> - 8.9.30-1.el5.fmi
- Fixed typecast problems in NFmiSmartToolIntepreter.cpp

* Mon Sep 29 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.29-1.el5.fmi
- Newbase header change forced rebuild

* Mon Sep 22 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.22-1.el5.fmi
- A few updates by Marko

* Thu Sep 11 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.11-1.el5.fmi
- A few API updates

* Tue Jul 15 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.7.15-1.el5.fmi
- Compiled with newbase 8.7.15-1

* Tue Mar 11 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.3.11-1.el5.fmi
- Linked against 8.3.11-1

* Mon Feb 18 2008 pkeranen <pekka.keranen@fmi.fi> - 8.2.18-1.el5.fmi
- Linked against newbase 8.2.18-1 with new WeatherAndCloudinessParam

* Wed Jan 30 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.1.30-1.el5.fmi
- Linked against newbase 8.1.25-3 with new parameter names

* Thu Dec 27 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.9-1.el5.fmi
- Linked against newbase-1.0.7-1 with fixed wind interpolation

* Wed Dec 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.8-1.el5.fmi
- Linked against newbase-1.0.6-1
- Some changes by Marko related to Q2 server

* Fri Nov 30 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.7-1.el5.fmi
- Marko added new features

* Mon Nov 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.7-1.el5.fmi
- Linked with newbase 1.0.4-1

* Thu Nov 15 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.6-1.el5.fmi
- Linked with newbase 1.0.3-1

* Thu Oct 18 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.5-1.el5.fmi
- New improvements from Marko Pietarinen

* Mon Sep 24 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.4-1.el5.fmi
- Fixed "make depend".

* Fri Sep 14 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.3-1.el5.fmi
- Added "make tag" feature

* Thu Sep 13 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.2-1.el5.fmi
- Improved make system plus new code from Pietarinen

* Thu Jun  7 2007 tervo <tervo@xodin.weatherproof.fi> - 
- Initial build.
