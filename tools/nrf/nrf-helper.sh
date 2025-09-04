#!/bin/bash

baud=115200
FILE=nrf.csv

function help {
echo " ./nrf-helper.sh"
echo "     -i, --info         print out device information ID/MAC/SN"
echo "         -s                  generate a save of ID/MAC/SN"
echo "         -d                  output a deployment mapping of ID/MAC (FIXME: Also need to use -s to generate the uuids!)"
echo "     -t                  output serial to terminal"
echo "     -clean              clean the logfile folder"
echo "     --l=*               save logs to folder"
echo "     --help -h           this help menu"
echo ""
exit 1
}


PARAMS=""
while (( "$#" )); do
  case "$1" in
    -clean)
      CLEAN=1
      shift
      ;;
    -i)
      INFO=1
      shift
      ;;
    --info)
      INFO=1
      shift
      ;;
    -s)
      SAVE=1
      shift
      ;;
    -d)
      DEPLOYMENT=1
      shift
      ;;
    --l=*)
      LOGS=${1:4}
      shift
      ;;
    -t)
      OUT=1
      shift
      ;;
    -u)
      USB=1
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

# --------------------------------------------------------------------------- #
# Save
# --------------------------------------------------------------------------- #
# Initialize arrays
id_arr=()
mac_arr=()
sn_arr=()

if [ -n "${SAVE+x}" ]; then
  echo "> WARN: Generating new \"$FILE\"!"
  rm -f "$FILE"
elif [ -f "$FILE" ]; then
  echo "> WARN: Read from \"$FILE\" (list of node id/mac/sn/port)..."
  while IFS=',' read -r id mac sn; do
    id_arr+=("$id")
    mac_arr+=("$mac")
    sn_arr+=("$sn")
  done < "$FILE"
else
  echo "> WARN: \"$FILE\" does not exist!"
fi

# --------------------------------------------------------------------------- #
# Board info
# --------------------------------------------------------------------------- #
if [ -n "${INFO+x}" ]; then
  echo "> Print board info..."

  if [ -n "${DEPLOYMENT+x}" ]; then
    DEPLOYMENT_FOLDER=../../os/services/deployment/nulltb/
    DEPLOYMENT_FILE=$DEPLOYMENT_FOLDER/deployment-map-nulltb.c
    DEPLOYMENT_HEADER_FILE=$DEPLOYMENT_FOLDER/deployment-map-nulltb.h
    mkdir -p $DEPLOYMENT_FOLDER
    if [[ -f "$DEPLOYMENT_FILE" ]]; then
      rm $DEPLOYMENT_FILE
    fi
    if [[ -f "$DEPLOYMENT_HEADER_FILE" ]]; then
      rm $DEPLOYMENT_HEADER_FILE
    fi
    echo "> Create deployment mapping in $DEPLOYMENT_FILE"
    echo ""
    : > $DEPLOYMENT_FILE
    echo "#include \"services/deployment/deployment.h\"" | tee -a $DEPLOYMENT_FILE
    echo "" | tee -a $DEPLOYMENT_FILE
    echo "#if CONTIKI_TARGET_NRF52840" | tee -a $DEPLOYMENT_FILE
    echo "const struct id_mac deployment_nulltb[] = {" | tee -a $DEPLOYMENT_FILE
  fi

  # nrfjprog exists so we can get mac addresses
  if command -v nrfjprog &> /dev/null; then
    len=`expr ${#mac_arr[@]} - 1`
    if [ $len -lt 0 ]; then
      len=-1
    fi
    id=0
    # Use a temporary file to store the output for deployment mapping
    if [ -n "${DEPLOYMENT+x}" ]; then
      temp_file=$(mktemp)
    fi
    
    nrfjprog --com | while read row; do
      port=${row:13:12}
      sn=${row::9}
      mac=`nrfjprog --snr $sn --memrd 0x100000A4 --n 8 | cut -c 13-21,28-30`
      mac="F4CE36${mac:9:2}${mac:6:2}${mac:4:2}${mac:2:2}${mac:0:2}"
      if [ -n "${SAVE+x}" ]; then
        id=$(( $id + 1 ))
        echo "$id,$mac,$sn" >> $FILE
      else
        if [ $len -ge 0 ]; then
          for i in $(seq 0 $len); do
            if [ "${mac_arr[$i]}" == "$mac" ]; then
              id=${id_arr[$i]}
            fi
          done
        fi
      fi
      if [ -n "${DEPLOYMENT+x}" ]; then
        # Store the MAC address for later processing with sequential IDs
        echo "$mac" >> "$temp_file"
      else
        echo "  - Port: $port | Node ID: $id | MAC Addr: $mac | JLink SN: $sn"
      fi
    done
    
    # Process deployment mapping with sequential IDs
    if [ -n "${DEPLOYMENT+x}" ]; then
      id=1
      deployment_count=0
      while read mac; do
        mac_lower=$(echo "$mac" | tr '[:upper:]' '[:lower:]')
        mac_formatted="0x${mac_lower:0:2},0x${mac_lower:2:2},0x${mac_lower:4:2},0x${mac_lower:6:2},0x${mac_lower:8:2},0x${mac_lower:10:2},0x${mac_lower:12:2},0x${mac_lower:14:2}"
        echo "  {   $id, {{$mac_formatted}} }," | tee -a $DEPLOYMENT_FILE
        id=$(( $id + 1 ))
        deployment_count=$(( deployment_count + 1 ))
      done < "$temp_file"
      rm "$temp_file"
      # Create/update header file with mapping length
      echo "#ifndef DEPLOYMENT_MAP_NULLTB_H_" >> $DEPLOYMENT_HEADER_FILE
      echo "#define DEPLOYMENT_MAP_NULLTB_H_" >> $DEPLOYMENT_HEADER_FILE
      echo "" >> $DEPLOYMENT_HEADER_FILE
      echo "#define DEPLOYMENT_MAPPING_LEN $deployment_count" >> $DEPLOYMENT_HEADER_FILE
      echo "" >> $DEPLOYMENT_HEADER_FILE
      echo "#endif /* DEPLOYMENT_MAP_NULLTB_H_ */" >> $DEPLOYMENT_HEADER_FILE
    fi
  #nrfjprog doesn't exist, so only get jlink sn
  else
    echo "WARN: nrfjprog not present so can't get MAC, but we can get JLink serial numbers!"
    len=`expr ${#sn_arr[@]} - 1`
    if [ $len -lt 0 ]; then
      len=-1
    fi
    for port in /dev/ttyACM*; do
      sn=`udevadm info -q property -a -p $(udevadm info -q path -n $port) | grep serial | grep -oP '(?<=").*(?=")' | grep "0006" | cut -c 4-`
      id="UNKNOWN"
      mac="UNKNOWN"
      if [ $len -ge 0 ]; then
        for i in $(seq 0 $len); do
          if [ "${sn_arr[$i]}" == "$sn" ]; then
            id=${id_arr[$i]}
          fi
        done
      fi
      echo "  - Port: $port | Node ID: $id | MAC Addr: $mac | JLink SN: $sn"
    done
  fi

  if [ -n "${DEPLOYMENT+x}" ]; then
    echo "  {   0, {{0}}}" | tee -a $DEPLOYMENT_FILE
    echo "};" | tee -a $DEPLOYMENT_FILE
    echo "#else" | tee -a $DEPLOYMENT_FILE
    echo "#warning \"WARN: Unknown DEPLOYMENT target\"" | tee -a $DEPLOYMENT_FILE
    echo "#endif" | tee -a $DEPLOYMENT_FILE
    echo ""
  fi

  exit 1
fi

# --------------------------------------------------------------------------- #
# Output to terminal/logs
# --------------------------------------------------------------------------- #
if [ -n "${OUT+x}" ]; then
  if [ -n "${LOGS+x}" ]; then
    echo "> Serial out to terminal... Logs sent to: $LOGS"
  else
    echo "> Serial out to terminal..."
  fi
  len=`expr ${#sn_arr[@]} - 1`
  mkdir -p ~/logs
  if [ -n "${CLEAN+x}" ]; then
    if [ -d ~/logs ]; then rm -Rf ~/logs; fi
    mkdir -p ~/logs
  fi
  nrfjprog --com | while read row; do
    port=${row:13:12}
    sn=`udevadm info -q property -a -p $(udevadm info -q path -n $port) | grep serial | grep -oP '(?<=").*(?=")' | grep "0006" | cut -c 4-`
    for i in $(seq 0 $len); do
      if [ ${sn_arr[$i]} == $sn ]; then
        id=${id_arr[$i]}
        mac=${mac_arr[$i]}
      fi
    done
    echo "  - Port: $port | Node ID: $id | MAC Addr: $mac | JLink SN: $sn"
    if [ -n "${LOGS+x}" ]; then
      CMD="socat $port,b$baud,raw,echo=0,nonblock STDOUT | ts [%Y-%m-%d\ %H:%M:%.S] | tee ~/logs/log_$id.txt"
    else
      CMD="socat $port,b$baud,raw,echo=0,nonblock STDOUT | ts [%Y-%m-%d\ %H:%M:%.S]"
    fi
    tilix -t "$x" -a session-add-down --focus-window -e "bash -c '$CMD'"
  done
  wait
fi

# --------------------------------------------------------------------------- #
# Connect to USB serial ports
# --------------------------------------------------------------------------- #
function arraydiff() {
  awk 'BEGIN{RS=ORS=" "}
       {NR==FNR?a[$0]++:a[$0]--}
       END{for(k in a)if(a[k])print k}' <(echo -n "${!1}") <(echo -n "${!2}")
}

if [ -n "${USB+x}" ]; then
  # get list of all acm ports
  acm_arr=()
  for port in /dev/ttyACM*; do
    acm_arr+=($port)
  done
  # get list of nrfjrpog ports
  njp_arr=()
  while read row; do
    njp_arr+=(${row:13:12})
  done <<< "$(nrfjprog --com)"

  usb_ports=($(arraydiff acm_arr[@] njp_arr[@]))

  for port in "${usb_ports[@]}"; do
  echo "  - Port: $port"
    # CMD="socat $port,b$baud,raw,echo=0,nonblock STDOUT | ts [%Y-%m-%d\ %H:%M:%.S]"
    CMD="picocom -fh -b $baud -c --imap lfcrlf $port"
    tilix -t "$x" -a session-add-down --focus-window -e "bash -c '$CMD'"
  done
  wait
fi
