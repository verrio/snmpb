#!/bin/bash
#
# Script used to build an SnmpB MAC installer.
# SnmpB needs to be compiled first, this script assumes
# that the application is already built.
#
# To compile snmpB on MAC
# 1- Download and install XCODE from here:
#    http://developer.apple.com/technology/xcode.html
# 2- Install QT for MAC. Download it from here: 
#    http://www.qtsoftware.com/downloads 
# 3- type 'make' in the snmpb root folder (trunk).
#
# To build the installer, execute this script locally,
# assuming XCODE is installed in the system.
#
# TODO:
#   Have binary run in setuid mode (for trap port binding)
#
# Martin Jolicoeur, March 2009
#

VERSION=1.0
APPNAME=SnmpB
APFILESPATH=${APPNAME}.app/Contents/MacOS

# Cleanup
rm -rf S* s*

# Copy application binary in local folder
cp -R ../../app/snmpb.app ./${APPNAME}.app
mv ${APFILESPATH}/snmpb ${APFILESPATH}/${APPNAME} 
sed -e "s/<string>snmpb<\/string>/<string>$APPNAME<\/string>/g" ${APPNAME}.app/Contents/Info.plist | grep -v "/dict" | grep -v "/plist" > Sinfo
cat fileassoc.txt >> Sinfo
echo "</dict>" >> Sinfo
echo "</plist>" >> Sinfo
mv -f Sinfo ${APPNAME}.app/Contents/Info.plist

# Copy and cleanup mib files to local folder
mkdir ${APFILESPATH}/mibs
mkdir ${APFILESPATH}/pibs
cp -R ../../libsmi/mibs/iana/* ${APFILESPATH}/mibs/
cp -R ../../libsmi/mibs/ietf/* ${APFILESPATH}/mibs/
cp -R ../../libsmi/mibs/tubs/* ${APFILESPATH}/mibs/
cp -R ../../libsmi/pibs/ietf/* ${APFILESPATH}/pibs/
cp -R ../../libsmi/pibs/tubs/* ${APFILESPATH}/pibs/
rm ${APFILESPATH}/mibs/Makefile* ${APFILESPATH}/pibs/Makefile* ${APFILESPATH}/pibs/*-orig
for i in `ls -1 ${APFILESPATH}/mibs/*`; do mv $i $i.mib; done
for i in `ls -1 ${APFILESPATH}/pibs/*`; do mv $i $i.pib; done

# Copy license file in local resource folder
mkdir -p ${APPNAME}.app/Contents/Resources/en.lproj
cp ../../license.txt ${APPNAME}.app/Contents/Resources/en.lproj/License

# Create a file-system image (.dmg) with the installer
macdeployqt ${APPNAME}.app -dmg
mv ${APPNAME}.dmg ${APPNAME}-${VERSION}.dmg

