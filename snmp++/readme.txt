SNMP++ 2.6 For HP UNIX Source Code and Examples:
====================================================================
Included within this package can be found the source code and  
examples for SNMP++ / HPUX. The following represents the directories which
are included within the compressed tar file and their contents.

For more details on the API , please refer to the API specification.

This library is a complete implementation of SNMP++ and does not
require other SNMP libraries to be present.  

Required Tools:
---------------------------------------------------------------------
HPUX 9.0 / 10.0
HPUX C++ Compiler


readme.txt ( this file)
|
|
|------ src ( .cpp files and Makefile for building libraries )
|
|------ include ( .h files for API and building libraries )
|
|------ consoleExamples ( a variety of console apps, .cpp and Makefile included )
|
|------ XExample ( a simple X11 Motif example with source and Makefile )


src Directory Contents:
--------------------------------------------------------------------
Makefile             - make file for HPUX build
address.cpp          - Address class source
asn1.cpp             - ASN1 encoding and decoding code. Based on CMU code.
collect.cpp          - Collection class source
counter.cpp          - Counter32 class source
ctr64.cpp            - Counter64 class source
eventlist.cpp        - UX event handler source
gauge.cpp            - Gauge32 class source
integer.cpp          - Integer32 class source
ipresolv.cpp         - UX Ip address resolver source
msec.cpp             - UX internal implementation support class 
msgqueue.cpp         - UX internal implementation support class
notifyqueue.cpp      - UX internal implementation support class 
octet.cpp            - Octet String class source
oid.cpp              - Oid class source
oidname.cpp          - UX internal implementation support class
pdu.cpp              - Pdu class source
snmpmsg.cpp          - SNMP Message class source
target.cpp           - SnmpTarget class source
test_app.cpp         - test application source
timetick.cpp         - TimeTicks class source
userdefined.cpp      - UX internal implementation support class
usertimeout.cpp      - UX internal implementation support class
uxsnmp.cpp           - UX internal implementation support class
vb.cpp               - Variable Binding class source


include Directory Contents:
---------------------------------------------------------------------
address.h            - Address classes definitions
asn1.h               - interfaces for ASN1 libraries
collect.h            - Collection class definitions 
counter.h            - Counter32 class definitions
ctr64.h              - Counter64 class definitions
eventlist.h          - UX internal implementation support class 
gauge.h              - Gauge32 class definition
integer.h            - Integer32 class definition
ipresolv.h           - UX internal implementation support class
msec.h               - UX internal implementation support class
msgqueue.h           - UX internal implementation support class
notifyqueue.h        - UX internal implementation support class
octet.h              - Octet String class definition
oid.h                - Oid class definition
oid_def.h            - UX internal implementation support class
pdu.h                - Pdu class definitions
smi.h                - SMI definitions
smival.h             - SnmpSyntax class definitions
snmp_pp.h            - SNMP++ main header file ( all one needs to include is this )
snmperrs.h           - SNMP++ error messages
snmpmsg.h            - SNMP Message class definition
target.h             - SnmpTarget class
timetick.h           - TimeTicks class
userdefined.h        - UX internal implementation support class  
usertimeout.h        - UX internal implementation support class
vb.h                 - Variable Binding class definition

consoleExamples Directory Contents:
---------------------------------------------------------------------
Makefile             - make file for building console apps
snmpBulk.cpp         - source for SNMP get bulk program
snmpGet.cpp          - source for SNMP get program
snmpNext.cpp         - source for SNMP get Next program
snmpSet.cpp          - source for SNMP set program
snmpTrap.cpp         - source for SNMP trap send program
snmpWalk.cpp         - source for SNMP walk program ( uses getnext for V1 or getBulk for v2)

XExample Directory Contents:
---------------------------------------------------------------------
Makefile            - Makefile for building X11 app
xmibform.cpp        - source     
xmibquery.cpp       - source
xmibform.h          - header file


Peter 

                              _____________ 
Peter Erik Mellquist         (    /        )
Software Design Engineer     |   /__   ___ | H E W L E T T
Hewlett Packard Company      |  /  /  /  / |
Workgroup Networks Division  | /  /  /__/  | P A C K A R D
Core Technologies            (______/______)
8000 Foothills Blvd
Roseville, CA 95747
916-785-8285
mellqust@hprnd.rose.hp.com






