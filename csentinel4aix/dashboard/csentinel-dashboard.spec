Name:           csentinel-dashboard
Version:        1.0.0
Release:        1.librepower
Summary:        C-Sentinel Web Dashboard for AIX
License:        MIT
Group:          Applications/System
URL:            https://gitlab.com/librepower/aix
Packager:       LibrePower <info@librepower.org>
Vendor:         LibrePower
BuildArch:      noarch
BuildRoot:      %{_tmppath}/%{name}-%{version}-root

%description
Web dashboard for C-Sentinel system monitoring. Provides real-time
visualization of system fingerprints across multiple AIX hosts.

%install
rm -rf $RPM_BUILD_ROOT
mkdir -p $RPM_BUILD_ROOT/opt/sentinel-dashboard/templates/admin
mkdir -p $RPM_BUILD_ROOT/opt/freeware/etc/nginx/conf.d
mkdir -p $RPM_BUILD_ROOT/opt/freeware/libexec/sentinel

cp /tmp/dashboard-rpm/SOURCES/app.py $RPM_BUILD_ROOT/opt/sentinel-dashboard/
cp /tmp/dashboard-rpm/SOURCES/migrate.sql $RPM_BUILD_ROOT/opt/sentinel-dashboard/
cp /tmp/dashboard-rpm/SOURCES/requirements.txt $RPM_BUILD_ROOT/opt/sentinel-dashboard/
cp /tmp/dashboard-rpm/SOURCES/templates/*.html $RPM_BUILD_ROOT/opt/sentinel-dashboard/templates/
cp /tmp/dashboard-rpm/SOURCES/templates/admin/*.html $RPM_BUILD_ROOT/opt/sentinel-dashboard/templates/admin/
cp /tmp/dashboard-rpm/SOURCES/nginx-sentinel.conf $RPM_BUILD_ROOT/opt/freeware/etc/nginx/conf.d/sentinel.conf
cp /tmp/dashboard-rpm/SOURCES/install-dashboard.sh $RPM_BUILD_ROOT/opt/freeware/libexec/sentinel/
chmod 755 $RPM_BUILD_ROOT/opt/freeware/libexec/sentinel/install-dashboard.sh

%files
%defattr(-,root,system)
/opt/sentinel-dashboard
/opt/freeware/etc/nginx/conf.d/sentinel.conf
/opt/freeware/libexec/sentinel

%post
echo ""
echo "C-Sentinel Dashboard installed!"
echo ""
echo "Setup: /opt/freeware/libexec/sentinel/install-dashboard.sh"
echo "Docs:  https://gitlab.com/librepower/aix/-/tree/main/csentinel4aix/dashboard"
echo ""
