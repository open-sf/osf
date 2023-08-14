baud=115200
FILE=nrf.csv

# This file requires installation of nrfjprog, socat and ts (which requires moreutils)

function help {
echo " ./nrf-helper.sh"
echo "     -info               print out device information ID/MAC/SN"
echo "         -d                  output to nulltb deployment mapping"
echo "         -a                  append to nulltb deployment mapping"
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
    -info)
      INFO=1
      shift
      ;;
    -d)
      DEPLOYMENT=1
      shift
      ;;
    -a)
      APPEND=1
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
# Board info
# --------------------------------------------------------------------------- #
if [[ -v INFO ]]; then
  echo "> Print board info..."

  DEPLOYMENT_FOLDER="../../os/services/deployment/nulltb"
  DEPLOYMENT_FILE="$DEPLOYMENT_FOLDER/deployment-map-nulltb.c"

  if [[ -v DEPLOYMENT ]]; then
    mkdir -p "$DEPLOYMENT_FOLDER"
    if [[ -f "$DEPLOYMENT_FILE" ]]; then
      rm "$DEPLOYMENT_FILE"
    fi
    echo "> Create deployment mapping in $DEPLOYMENT_FILE"
    echo ""
    echo "#include \"services/deployment/deployment.h\"" | tee -a "$DEPLOYMENT_FILE"
    echo "" | tee -a "$DEPLOYMENT_FILE"
    echo "#if CONTIKI_TARGET_NRF52840" | tee -a "$DEPLOYMENT_FILE"
    echo "const struct id_mac deployment_nulltb[] = {" | tee -a "$DEPLOYMENT_FILE"
  elif [[ -v APPEND ]]; then
    if [[ -f "$DEPLOYMENT_FILE" ]]; then
      echo "> Append to deployment mapping in $DEPLOYMENT_FILE"
    else
      echo "> ERROR: deployment does not exist! Please create with '-d' option."
    fi
  fi

  # nrfjprog exists so we can get mac addresses
  if command -v nrfjprog &> /dev/null; then
    len=`expr ${#mac_arr[@]} - 1`
    id=0
    #nrfjprog --com | while read row; do
    nrfjprog -i | while read row; do
      port=${row:13:12}
      sn=${row::9}
      mac=`nrfjprog --snr $sn --memrd 0x100000A4 --n 8 | cut -c 13-21,28-30`
      mac="F4CE36${mac:9:2}${mac:6:2}${mac:4:2}${mac:2:2}${mac:0:2}"
      id=$(($id + 1))
      if [[ -v DEPLOYMENT ]]; then
        mac=${mac,,}
        mac="0x${mac:0:2},0x${mac:2:2},0x${mac:4:2},0x${mac:6:2},0x${mac:8:2},0x${mac:10:2},0x${mac:12:2},0x${mac:14:2}"
        echo "  {   $id, {{$mac}} }," | tee -a $DEPLOYMENT_FILE
      elif [[ -v APPEND ]]; then
        mac=${mac,,}
        formatted_mac="0x${mac:0:2},0x${mac:2:2},0x${mac:4:2},0x${mac:6:2},0x${mac:8:2},0x${mac:10:2},0x${mac:12:2},0x${mac:14:2}"
        # Create a temporary file
        temp_file=$(mktemp)
        if grep -q "$formatted_mac" "$DEPLOYMENT_FILE"; then
          echo "> ERROR: This MAC address is already included in the deployment!"
          exit 1
        fi
        # Get the value from the last line before {   0, {{0}}}
        last_value=$(awk '/\{[[:space:]]+0,[[:space:]]\{\{0\}\}/{print prev}{prev=$0}' "$DEPLOYMENT_FILE" | awk -F',' '{print $1}' | awk -F'{' '{print $2}')
        next_value=$((last_value + 1))
        new_mac="  {   $next_value, {{${formatted_mac}}} },"
        while IFS= read -r line; do
          if [[ "$line" =~ \{[[:space:]]+0,[[:space:]]\{\{0\}\} ]]; then
            echo "$new_mac"
            echo "$new_mac" >> "$temp_file"
          fi
          echo "$line"
          echo "$line" >> "$temp_file"
        done < "$DEPLOYMENT_FILE"
        # Overwrite input file with the temp file
        mv "$temp_file" "$DEPLOYMENT_FILE"
      else
        echo "  - Port: $port | Node ID: $id | MAC Addr: $mac | JLink SN: $sn"
      fi
    done
  #nrfjprog doesn't exist, so only get jlink sn
  else
    echo "WARN: nrfjprog not present so can't get MAC, but we can get JLink serial numbers!"
    len=`expr ${#sn_arr[@]} - 1`
    for port in /dev/ttyACM*; do
      sn=`udevadm info -q property -a -p $(udevadm info -q path -n $port) | grep serial | grep -oP '(?<=").*(?=")' | grep "0006" | cut -c 4-`
      id="UNKNOWN"
      mac="UNKNOWN"
      for i in $(seq 0 $len); do
        if [ ${sn_arr[$i]} == $sn ]; then
          id=${id_arr[$i]}
        fi
      done
      echo "  - Port: $port | Node ID: $id | MAC Addr: $mac | JLink SN: $sn"
    done
  fi

  if [[ -v DEPLOYMENT ]]; then
    echo "  {   0, {{0}}}" | tee -a "$DEPLOYMENT_FILE"
    echo "};" | tee -a "$DEPLOYMENT_FILE"
    echo "#else" | tee -a "$DEPLOYMENT_FILE"
    echo "#warning \"WARN: Unknown DEPLOYMENT target\"" | tee -a "$DEPLOYMENT_FILE"
    echo "#endif" | tee -a "$DEPLOYMENT_FILE"
    echo ""
  fi

  exit 1
fi

# --------------------------------------------------------------------------- #
# Output to terminal/logs
# --------------------------------------------------------------------------- #
if [[ -v OUT ]]; then
  if [[ -v LOGS ]]; then
    echo "> Serial out to terminal... Logs sent to: $LOGS"
  else
    echo "> Serial out to terminal..."
  fi
  len=`expr ${#sn_arr[@]} - 1`
  mkdir -p ~/logs
  if [[ -v CLEAN ]]; then
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
    if [[ -v LOGS ]]; then
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

if [[ -v USB ]]; then
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
