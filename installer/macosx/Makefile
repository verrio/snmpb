VERSION=0.5
APFILESPATH=SnmpB.app/Contents/MacOS
#QTCOREDIR=`ls -l /Library/Frameworks/QtCore.framework/QtCore| sed -e 's/.*-> //g;s/QtCore//g'`
#QTGUIDIR=`ls -l /Library/Frameworks/QtGui.framework/QtGui| sed -e 's/.*-> //g;s/QtGui//g'`

all:
	rm -rf S* s*
	cp -R ../../app/snmpb.app ./SnmpB.app
	mv ${APFILESPATH}/snmpb ${APFILESPATH}/SnmpB 
	sed -e 's/<string>snmpb<\/string>/<string>SnmpB<\/string>/g' SnmpB.app/Contents/Info.plist > Sinfo
	mv -f Sinfo SnmpB.app/Contents/Info.plist
	mkdir ${APFILESPATH}/mibs
	mkdir ${APFILESPATH}/pibs
	cp -R ../../libsmi/mibs/iana/* ${APFILESPATH}/mibs
	cp -R ../../libsmi/mibs/ietf/* ${APFILESPATH}/mibs
	cp -R ../../libsmi/mibs/irtf/* ${APFILESPATH}/mibs
	cp -R ../../libsmi/mibs/tubs/* ${APFILESPATH}/mibs
	cp -R ../../libsmi/pibs/ietf/* ${APFILESPATH}/pibs
	cp -R ../../libsmi/pibs/tubs/* ${APFILESPATH}/pibs
	rm ${APFILESPATH}/mibs/Makefile* ${APFILESPATH}/pibs/Makefile*
#	mkdir -p ${APFILESPATH}/QtCore.framework/${QTCOREDIR}
#	cp -R /Library/Frameworks/QtCore.framework/${QTCOREDIR}QtCore ${APFILESPATH}/QtCore.framework/${QTCOREDIR}
#	mkdir -p ${APFILESPATH}/QtGui.framework/${QTGUIDIR}
#	cp -R /Library/Frameworks/QtGui.framework/${QTGUIDIR}QtGui ${APFILESPATH}/QtGui.framework/${QTGUIDIR}
	mkdir -p SnmpB.app/Contents/Resources/en.lproj
	cp ../../license.txt SnmpB.app/Contents/Resources/en.lproj/License
	/Developer/Applications/Utilities/PackageMaker.app/Contents/MacOS/PackageMaker -o SnmpB.mpkg -t SnmpB -l /Applications/SnmpB.app -b -g 10.4 -i Martin.Jolicoeur -n ${VERSION} -e SnmpB.app/Contents/Resources --root SnmpB.app
	hdiutil create SnmpB.dmg -volname "SnmpB" -fs HFS+ -srcfolder SnmpB.mpkg
