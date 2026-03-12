Name:           semeru-jdk25
Version:        25.0.2
Release:        1.librepower
Summary:        IBM Semeru Certified Edition JDK 25 for AIX ppc64
License:        IBM
URL:            https://developer.ibm.com/languages/java/semeru-runtimes/
Group:          Development/Languages

%description
IBM Semeru Certified Edition JDK 25 (Eclipse OpenJ9) for AIX ppc64.

Installed at /opt/freeware/lib/jvm/semeru-25 as an ADDITIONAL JDK.

This package does NOT:
  - Modify /usr/bin/java or /usr/java8_64
  - Change system JAVA_HOME or PATH
  - Interfere with any existing Java installation

To use it explicitly:
  export JAVA_HOME=/opt/freeware/lib/jvm/semeru-25
  export PATH=$JAVA_HOME/bin:$PATH

%install
rm -rf %{buildroot}
mkdir -p %{buildroot}/opt/freeware/lib/jvm/semeru-25
cp -R /tmp/jdk-25.0.2+10/* %{buildroot}/opt/freeware/lib/jvm/semeru-25/

%files
%defattr(-,root,system,-)
/opt/freeware/lib/jvm/semeru-25

%post
echo ""
echo "IBM Semeru JDK 25 installed at /opt/freeware/lib/jvm/semeru-25"
echo ""
echo "Your system Java is NOT affected:"
SYSJAVA=$(/usr/java8_64/bin/java -version 2>&1 | head -1) 2>/dev/null
echo "  System:  ${SYSJAVA:-not found}"
echo "  Semeru:  $(/opt/freeware/lib/jvm/semeru-25/bin/java -version 2>&1 | head -1)"
echo ""
echo "To use: export JAVA_HOME=/opt/freeware/lib/jvm/semeru-25"
echo ""

%changelog
* Wed Mar 12 2026 LibrePower <hello@librepower.org> - 25.0.2-1.librepower
- IBM Semeru Certified Edition JDK 25.0.2+10 (OpenJ9 0.57.0)
- Standalone installation, does not modify system Java
