#!/bin/bash

export PATH=$PWD:$PATH

# local node address & port
THENODEADDRESS=$1
THENODEPORT=$2

# check input parameters
if [ -z "$1" ] || [ -z "$2" ]; then
  echo "Usage : ./clear_stat.sh nodeaddr nodeport"
  # ./clear_stat.sh fd00::f6ce:3606:8f9f:f3d2 7777
  exit 1
fi

# max amount of nodes
NNODESMAX=255

# request amount of nodes
NNODES=$(echo -n "?NN" | nc -6u -w1 -W1 "$THENODEADDRESS" "$THENODEPORT")
#echo $? $NNODES
if [ "$?" -ne 0 ] || [ -z "$NNODES" ] || [ "${NNODES:0:3}" != "!NN" ]; then
    echo nc error : "$?" or "$NNODES"
    exit 1
fi
NNODES=${NNODES:3:3}; NNODES=${NNODES//[!0-9]/}
echo Total nodes : "$NNODES"


# request node_id of the connected node
THENODEID=$(echo -n "?NN0" | nc -6u -w1 -W1 "$THENODEADDRESS" "$THENODEPORT")
if [ "$?" -ne 0 ] || [ -z "$THENODEID" ] || [ "${THENODEID:0:3}" != "!NN" ]; then
    echo nc error : "$?" or "$THENODEID"
    exit 1
fi
THENODEID=${THENODEID:3:3}; THENODEID=${THENODEID//[!0-9]/}
echo The node_id : "$THENODEID"

# request node_id of the ISN node
ISNNODEID=$(echo -n "?NN255" | nc -6u -w1 -W1 "$THENODEADDRESS" "$THENODEPORT")
if [ "$?" -ne 0 ] || [ -z "$ISNNODEID" ] || [ "${ISNNODEID:0:3}" != "!NN" ]; then
    echo nc error : "$?" or "$ISNNODEID"
    exit 1
fi
ISNNODEID=${ISNNODEID:3:3}; ISNNODEID=${ISNNODEID//[!0-9]/}
echo ISN node_id : "${ISNNODEID}"

# request ipv6 address of the connected node
THENODEADDRESS1=$(echo -n "?NN${THENODEID}" | nc -6u -w1 -W1 "$THENODEADDRESS" "$THENODEPORT")
if [ "$?" -ne 0 ] || [ -z "$THENODEADDRESS1" ] || [ "${THENODEADDRESS1:0:3}" != "!NN" ]; then
    echo nc error : "$?" or "$THENODEADDRESS1"
    exit 1
fi
THENODEADDRESS1=${THENODEADDRESS1:3:32}
echo The node address : "$THENODEADDRESS1"
# prefix is hardcoded in firmware
PREFIX=${THENODEADDRESS1:0:2}
#echo ' '

# Get statistic counters on all Nodes
if [ "$NNODES" -lt "$NNODESMAX" ]; then
	COUNTER=1
	echo ' '
	while [ "$COUNTER" -le "$(("NNODES"))" ]; do       
		# get node address       	
   		NODEADDRESS=$(echo -n "?NN${COUNTER}" | nc -6u -w1 -W1 "$THENODEADDRESS" "$THENODEPORT")
   		if [ "$?" -ne 0 ] || [ -z "$NODEADDRESS" ] || [ "${NODEADDRESS:0:3}" != "!NN" ]; then
    			echo nc error : "$?" or "$NODEADDRESS"
    			exit 1
		fi
		NODEADDRESS=${NODEADDRESS:3:32}
		# get ping time
		PING=$(ping6 -c1 -W1 "$NODEADDRESS" | grep -oP '(?<=time\=).*')
		if [ "$?" -ne 0 ] || [ -z "$PING" ] ; then
    			PING="Offline"
    			RSSI="Offline"
    			SSTAT="Offline"
    			TSTAT="Offline"
    			ASTAT="Offline"
    		else
    			# ping OK, get RSSI
    			RSSI=$(echo -n "?RS" | nc -6u -w1 -W1 "$NODEADDRESS" "$THENODEPORT")
   			if [ "$?" -ne 0 ] || [ -z "$RSSI" ] || [ "${RSSI:0:3}" != "!RS" ]; then
    			RSSI="Offline"
			else
			RSSI=${RSSI:3:10}
			fi
    			# get STAT counters
    			# S round
			SSTAT=$(echo -n "?SS" | nc -6u -w1 -W1 "$NODEADDRESS" "$THENODEPORT")
   			if [ "$?" -ne 0 ] || [ -z "$SSTAT" ] || [ "${SSTAT:0:3}" != "!SS" ]; then
    				SSTAT="Offline"
			else
				SSTAT="${SSTAT:3:100}"
			fi
			# T round
			TSTAT=$(echo -n "?ST" | nc -6u -w1 -W1 "$NODEADDRESS" "$THENODEPORT")
   			if [ "$?" -ne 0 ] || [ -z "$TSTAT" ] || [ "${TSTAT:0:3}" != "!ST" ]; then
    				TSTAT="Offline"
			else
				TSTAT="${TSTAT:3:100}"
			fi
			# A round
			ASTAT=$(echo -n "?SA" | nc -6u -w1 -W1 "$NODEADDRESS" "$THENODEPORT")
   			if [ "$?" -ne 0 ] || [ -z "$ASTAT" ] || [ "${ASTAT:0:3}" != "!SA" ]; then
    				ASTAT="Offline"
			else
				ASTAT="${ASTAT:3:100}"
			fi			
		fi
		# print
		echo "$COUNTER" node address : "$NODEADDRESS"  ping : "$PING" rssi : "$RSSI"
		echo "$COUNTER" node SYNC round : "$SSTAT"
		echo "$COUNTER" node TX round   : "$TSTAT"
		echo "$COUNTER" node ACK round  : "$ASTAT"
		echo ' '
		sleep 0.1
        	COUNTER="$(("COUNTER"+1))"	 
	done
fi	
echo ' '
exit 0

