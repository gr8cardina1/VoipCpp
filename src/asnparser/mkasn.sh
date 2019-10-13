#!/bin/sh
for i in 2 3 4; do ./obj_FreeBSD_x86_r/asnparser -m H2${i}5 -i -c -n -r MULTIMEDIA-SYSTEM-CONTROL=H245 h2${i}5.asn; done
./obj_FreeBSD_x86_r/asnparser -m T38 -i -c -n -r MULTIMEDIA-SYSTEM-CONTROL=H245 t38.asn;