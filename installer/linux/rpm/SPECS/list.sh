#!/bin/bash

ls -1 ../../../../libsmi/mibs/* | grep -v "\.\." | grep -v "Makefile" | grep -v "^$" | sed -e 's/^/%prefix\/share\/apps\/snmpb\/mibs\//g' >> list
ls -1 ../../../../libsmi/pibs/* | grep -v "\.\." | grep -v "Makefile" | grep -v "^$" | sed -e 's/^/%prefix\/share\/apps\/snmpb\/pibs\//g' >> list

