#!/bin/bash -e

# Contiki directory
CONTIKI=$1

# Simulation file
BASENAME=$(basename $0 .sh)

./test-native-border-router.sh $CONTIKI $BASENAME fd00::204:4:4:4 60 1200 4
