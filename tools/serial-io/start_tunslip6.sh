#!/bin/bash

export PATH=$PWD:$PATH

# serial port
SERIALPORT=$1
#SERIALPORT=/dev/ttyACM4

# tundev
TUNDEV=$2
#TUNDEV=tun0

# check input parameters
if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage : ./start_tunslip6 suodev tundev"
  # ./start_tunslip6.sh /dev/ttyACM41 tun0
  exit 1
fi

# kill tunslip6 instanses
killall tunslip6
sleep 0.5
killall tunslip6
sleep 0.5
echo ' '

# max amount of nodes
NNODESMAX=255

# test connection to the node
NN=$(timeout --preserve-status 2 slipcmd  -sn -R?N "$SERIALPORT")
if [ "$?" -ne 0 ] || [ -z "$NN" ] ; then
    echo slipcmd error : "$?" or "$NN"
    #exit 1
fi
#echo $NN # test !N8�
#NN=${NN:2:3}; NN=${NN//[!0-9]/}
#echo $NN # test !N8�

# request amount of nodes
NNODES=$(timeout --preserve-status 2 slipcmd  -sn -R?N "$SERIALPORT")
#echo $? $NNODES
if [ "$?" -ne 0 ] || [ -z "$NNODES" ] ; then
    echo slipcmd error : "$?" or "$NNODES"
    exit 1
fi
#NNODES='!N254x' # test max
NNODES=${NNODES:2:3}; NNODES=${NNODES//[!0-9]/}
echo Total nodes : "$NNODES"

# request node_id of the connected node
THENODEID=$(timeout --preserve-status 2 slipcmd  -sn -R?N0 "$SERIALPORT")
if [ "$?" -ne 0 ] || [ -z "$THENODEID" ] ; then
    echo slipcmd error : "$?" or "$THENODEID"
    exit 1
fi
THENODEID=${THENODEID:2:3}; THENODEID=${THENODEID//[!0-9]/}
echo The node_id : "$THENODEID"

# request node_id of the ISN node
ISNNODEID=$(timeout --preserve-status 2 slipcmd  -sn -R?N255 "$SERIALPORT")
if [ "$?" -ne 0 ] || [ -z "$ISNNODEID" ] ; then
    echo slipcmd error : "$?" or "$ISNNODEID"
    exit 1
fi
ISNNODEID=${ISNNODEID:2:3}; ISNNODEID=${ISNNODEID//[!0-9]/}
echo ISN node_id : "${ISNNODEID}"

# request ipv6 address of the connected node
THENODEADDRESS=$(timeout --preserve-status 2 slipcmd  -sn -R?N"${THENODEID}" "$SERIALPORT")
if [ "$?" -ne 0 ] || [ -z "$THENODEADDRESS" ] ; then
    echo slipcmd error : "$?" or "$THENODEADDRESS"
    exit 1
fi
THENODEADDRESS=${THENODEADDRESS:2:32}
echo The node address : "$THENODEADDRESS"
echo ' '

# start tunslip6
# THENODEID=250 # test
# prefix is hardcoded in firmware
PREFIX=${THENODEADDRESS:0:2}
# echo $PREFIX
THEHOSTPREFIX=$PREFIX$(printf '%02x\n' "$THENODEID")

# start tunslip6 in backgroud
echo tunslip6 "$THEHOSTPREFIX"::1/64 -s "$SERIALPORT" -t "$TUNDEV" -d1 -H -v0 -B 921600 "$3" "$4" "$5" "$6"# test
tunslip6 "$THEHOSTPREFIX"::1/64 -s "$SERIALPORT" -t "$TUNDEV" -B 921600 -d1 -H -v0 $3 $4 $5 $6&
if [ "$?" -ne 0 ] ; then
    echo tunslip6 error : "$?"
    exit 1
fi
sleep 1
ifconfig -s "$TUNDEV" #test

# Is this option part of tunslip6 executable ?
sysctl -w net.ipv6.conf.all.forwarding=1

# create route table to all Hosts
if [ "$NNODES" -lt "$NNODESMAX" ]; then
COUNTER=0
echo ' '
	while [ "$COUNTER" -le "$(("NNODES"))" ]; do
        # skip route for the connected node
        if [[ "$THENODEID" != "$COUNTER" ]]; then
        	# add route to other host        	
        	OTHERHOSTADDR=$PREFIX$(printf '%02x\n' "$COUNTER")
        	echo ip route add "$OTHERHOSTADDR"::/64 via "$THEHOSTPREFIX""${THENODEADDRESS:4:32}" dev "$TUNDEV" # test
        	ip route add "$OTHERHOSTADDR"::/64 via "$THEHOSTPREFIX""${THENODEADDRESS:4:32}" dev "$TUNDEV"        	
        	if [ "$?" -ne 0 ] ; then
    		echo ip route add error : "$?"
    		exit 1
		fi
		sleep 0.1
        fi
        COUNTER="$(("COUNTER"+1))"	 
	done
fi
echo ' '; ifconfig -s "$TUNDEV" ;echo ' ';ip -6 route # test
exit 0

