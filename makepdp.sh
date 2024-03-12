#!/bin/bash

PDPSRC=pdp
PDPOUT=pdp-out

rm -rf $PDPOUT
mkdir $PDPOUT
cp -r $PDPSRC/* $PDPOUT

cp -r include $PDPOUT/
cp src/ld.c src/nm.c src/rel.c src/size.c $PDPOUT/
cp src/as/*.c $PDPOUT/as/
cp src/as/*.h $PDPOUT/as/