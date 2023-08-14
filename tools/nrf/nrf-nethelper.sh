#!/bin/bash

FILE=nodes.csv

function help {
  echo " ./connect-host.sh"
  echo "     -g                  generate deployment of nrf macs using nrfutil"
  echo "     -ipv6               create static routes from deployment"
  echo "     -clean              cleanup interfaces and routes"
  echo "     -ping               ping nrf nodes"
  echo "     --help -h           this help menu"
  echo ""
  exit 1
}

PARAMS=""
while (( "$#" )); do
  case "$1" in
    -g)
      MAP=1
      shift
      ;;
    -ipv6)
      IPV6=1
      shift
      ;;
    -clean)
      CLEAN=1
      shift
      ;;
    -ping)
      PING=1
      shift
      ;;
    -iperf)
      IPERF=1
      shift
      ;;
    --help)
      help
      shift 2
      ;;
    -h)
      help
      shift 2
      ;;
    *) # preserve positional arguments
      PARAMS="$PARAMS $1"
      shift
      ;;
  esac
done

function menu_from_array()
{
  select choice; do
    # Check the selected menu item number
    if [ 1 -le "$REPLY" ] && [ "$REPLY" -le $# ]; then
      break;
    else
      echo "Wrong selection: Select any number from 1-$#"
    fi
  done
}

function generate_mapping() {
  # nrfjprog exists so we can get mac addresses
  if command -v nrfjprog &> /dev/null; then
    id=0
    nrfjprog --com | while read row; do
      port=${row:13:12}
      sn=${row::9}
      mac=`nrfjprog --snr $sn --memrd 0x100000A4 --n 8 | cut -c 13-21,28-30`
      mac="f6ce:36${mac:9:2}:${mac:6:2}${mac:4:2}:${mac:2:2}${mac:0:2}"
      mac=${mac,,}
      id=$(( $id + 1 ))
      echo "  - Port: $port | Node ID: $id | MAC Addr: $mac | JLink SN: $sn"
      echo "$id,$mac" >> $FILE
    done
  #nrfjprog doesn't exist!
  else
    echo "WARN: nrfjprog not present so can't get MAC!"
    exit 1
  fi
}

function check_deployment_file() {
  if [ ! -f "$FILE" ]; then
    echo "ERROR: \"$FILE\" does not exist!"
    echo "Use '-g' to generate mapping from nrfutil."
    exit 1
  else
    echo "Reading from \"$FILE\" (csv of id,mac)..."
    readarray -t id_arr < <(cut -d, -f1 $FILE)
    readarray -t mac_arr < <(cut -d, -f2 $FILE)
  fi
}

# --------------------------------------------------------------------------- #
# Generate mapping file
# --------------------------------------------------------------------------- #
if [[ -v MAP ]]; then
  if [[ -f "$FILE" ]]; then
    rm $FILE
  fi
  echo "> Create deployment mapping in ./$FILE..."
  generate_mapping
  echo "Finished writing to $FILE!"
  exit 1
fi

# --------------------------------------------------------------------------- #
# Connect host through IPv6 routes
# --------------------------------------------------------------------------- #
if [[ -v IPV6 ]]; then
  # if [ -z "$interfaces_arr" ]; then
  #   echo "ERROR: No tun interfaces detected!"
  #   exit 1
  # fi
  # Elevate privledges
  if [ $EUID != 0 ]; then
    sudo "$0" "$@"
  fi

  # Check the deployment file exists and create mac and id arrays
  check_deployment_file

  echo "Select border router device..."
  menu_from_array "${mac_arr[@]}"
  device_mac=$choice
  device_id=`printf %02d ${id_arr[$REPLY-1]}`
  device_id_16=`printf %x $device_id`

  echo "Select serial port..."
  ports_arr=($(ls /dev/ttyACM*))
  menu_from_array "${ports_arr[@]}"
  device_port=$choice

  echo "> Run tunslip6 in background..."
  CMD="../serial-io/tunslip6 2001:db8:${device_id_16}::1000/64 -s $device_port -B 921600 -d1"
  sudo $CMD &
  TUNPID=$!
#echo "$CMD"
  # tilix -t "$x" -a session-add-down -e "bash -c '$CMD'"
  sleep 7

  # Get tundev interface name
  interfaces_arr=($(ip link | awk -F: '$0 ~ "tun"{print $2}'))
  len=`expr ${#interfaces_arr[@]} - 1`
  tundev=tun$len
  echo "$tundev"


  echo "> Enable IPv6 forwarding"
  sudo sysctl -w net.ipv6.conf.all.forwarding=1

  len=`expr ${#id_arr[@]} - 1`

  if [[ $device_id -le 2 ]]; then
    prefix=4
  elif [[ $device_id -lt 80 ]]; then
    prefix=2
  else 
    prefix=3
  fi
  echo "$prefix"

  echo "> Add (static) routes to mesh via $tundev"
  sudo ip route add fd0${prefix}::/64 via 2001:db8:${device_id_16}::${device_id_16} dev $tundev
  echo "sudo ip route add fd0${prefix}::/64 via 2001:db8:${device_id_16}::${device_id_16} dev $tundev"
  echo "> Add (static) routes to other hosts via $tundev"
  for i in $(seq 0 $len); do
    if [ ${mac_arr[$i]} != $device_mac ]; then
      route_id=`printf %02x ${id_arr[$i]}`
      sudo ip route add 2001:db8:$route_id::/64 via 2001:db8:$device_id_16::${device_id_16} dev $tundev
      echo "sudo ip route add 2001:db8:$route_id::/64 via 2001:db8:$device_id_16::${device_id_16} dev $tundev"
    fi
  done
  sudo ip -6 route add 2001:db8:1000::/64 dev $tundev
  echo "sudo ip -6 route add 2001:db8:1000::/64 dev $tundev"

  wait $TUNPID

fi

# --------------------------------------------------------------------------- #
# Clean interfaces and routes
# --------------------------------------------------------------------------- #
if [[ -v CLEAN ]]; then
  interfaces_arr=($(ip link | awk -F: '$0 ~ "tun"{print $2}'))
  len=`expr ${#interfaces_arr[@]}`
  for i in $(seq 0 $len); do
    ifconfig tun$i down
    netstat -nr | awk '{ if ($2 == "tun$i") print "route delete -net "$1; }' | sh
  done
fi

# --------------------------------------------------------------------------- #
# Test PING
# --------------------------------------------------------------------------- #
if [[ -v PING ]]; then
  echo "> ping6 all nodes in deployment"

  check_deployment_file

  len=`expr ${#id_arr[@]} - 1`
  for i in $(seq 0 $len); do
    ping6 fd01::${mac_arr[$i]} -c 5
  done
fi

# --------------------------------------------------------------------------- #
# Test IPERF
# --------------------------------------------------------------------------- #
if [[ -v IPERF ]]; then

  echo "> iperf3 to all fdXX::/64 hosts we have routes for"

  check_deployment_file

  # run the iperf server as a background process
  trap 'kill $(jobs -pr)' SIGINT SIGTERM EXIT
  iperf3 -s &

  len=`expr ${#id_arr[@]} - 1`
  for i in $(seq 0 $len); do
    iperf3 -6 -u -c fd0${id_arr[$i]}::1 -t 10
  done
fi
