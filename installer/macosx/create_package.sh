#!/bin/bash
#
# http://developer.apple.com/TOOLS/xcode/
#
#
#
#

VERSION=0.5
APPNAME=SnmpB
APFILESPATH=${APPNAME}.app/Contents/MacOS

# Cleanup
rm -rf S* s*

# Copy application binary in local folder
cp -R ../../app/snmpb.app ./${APPNAME}.app
mv ${APFILESPATH}/snmpb ${APFILESPATH}/${APPNAME} 
sed -e 's/<string>snmpb<\/string>/<string>${APPNAME}<\/string>/g' ${APPNAME}.app/Contents/Info.plist > Sinfo
mv -f Sinfo ${APPNAME}.app/Contents/Info.plist

# Copy and cleanup mib files to local folder
mkdir ${APFILESPATH}/mibs
mkdir ${APFILESPATH}/pibs
cp -R ../../libsmi/mibs/iana/* ${APFILESPATH}/mibs
cp -R ../../libsmi/mibs/ietf/* ${APFILESPATH}/mibs
cp -R ../../libsmi/mibs/irtf/* ${APFILESPATH}/mibs
cp -R ../../libsmi/mibs/tubs/* ${APFILESPATH}/mibs
cp -R ../../libsmi/pibs/ietf/* ${APFILESPATH}/pibs
cp -R ../../libsmi/pibs/tubs/* ${APFILESPATH}/pibs
rm ${APFILESPATH}/mibs/Makefile* ${APFILESPATH}/pibs/Makefile*

# Copy QT libs in local folder,
#QTCOREDIR=`ls -l /Library/Frameworks/QtCore.framework/QtCore| sed -e 's/.*-> //g;s/QtCore//g'`
#QTGUIDIR=`ls -l /Library/Frameworks/QtGui.framework/QtGui| sed -e 's/.*-> //g;s/QtGui//g'`
#
#mkdir -p ${APFILESPATH}/QtCore.framework/${QTCOREDIR}
#cp -R /Library/Frameworks/QtCore.framework/${QTCOREDIR}QtCore ${APFILESPATH}/QtCore.framework/${QTCOREDIR}
#mkdir -p ${APFILESPATH}/QtGui.framework/${QTGUIDIR}
#cp -R /Library/Frameworks/QtGui.framework/${QTGUIDIR}QtGui ${APFILESPATH}/QtGui.framework/${QTGUIDIR}

# Copy license file in local resource folder
mkdir -p ${APPNAME}.app/Contents/Resources/en.lproj
cp ../../license.txt ${APPNAME}.app/Contents/Resources/en.lproj/License

# Create meta-package
/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -o ${APPNAME}.mpkg -t ${APPNAME} -l /Applications/${APPNAME}.app -b -g 10.4 -i Martin.Jolicoeur -n ${VERSION} -e ${APPNAME}.app/Contents/Resources --root ${APPNAME}.app

# Create a file-system image (.dmg) with the installer
hdiutil create ${APPNAME}.dmg -volname "${APPNAME}" -fs HFS+ -srcfolder ${APPNAME}.mpkg

