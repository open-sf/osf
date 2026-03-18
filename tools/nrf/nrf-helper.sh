#!/bin/bash

baud=115200
FILE=nrf.csv

UNAME_S="$(uname -s 2>/dev/null || echo unknown)"
IS_MAC=0
if [[ "$UNAME_S" == "Darwin" ]]; then
  IS_MAC=1
fi

function spawn_terminal() {
  local cmd="$1"
  local title="${2:-nrf-helper}"

  if [[ $IS_MAC -eq 1 ]]; then
    # macOS: prefer iTerm2, fallback to Apple Terminal
    # Override with NRF_TERMINAL=terminal or NRF_TERMINAL=iterm2
    local esc
    esc="$(printf '%s' "$cmd" | sed 's/\\/\\\\/g; s/"/\\\\\\"/g')"

    if [[ "${NRF_TERMINAL:-}" != "terminal" ]] && (open -Ra "iTerm" >/dev/null 2>&1 || open -Ra "iTerm2" >/dev/null 2>&1); then
      osascript >/dev/null 2>&1 <<EOF
tell application "iTerm2"
  activate
  if (count of windows) = 0 then
    set w to (create window with default profile)
  else
    set w to current window
  end if
  tell w
    set t to (create tab with default profile)
    tell current session of t
      write text "bash -lc \\"$esc\\""
    end tell
  end tell
end tell
EOF
      return $?
    fi

    osascript >/dev/null 2>&1 <<EOF
tell application "Terminal"
  activate
  do script "bash -lc \\"$esc\\""
end tell
EOF
    return $?
  fi

  if command -v tilix >/dev/null 2>&1; then
    tilix -t "$title" -a session-add-down --focus-window -e "bash -lc '$cmd'"
    return $?
  fi
  if command -v gnome-terminal >/dev/null 2>&1; then
    gnome-terminal -- bash -lc "$cmd"
    return $?
  fi
  if command -v xterm >/dev/null 2>&1; then
    xterm -T "$title" -e bash -lc "$cmd" &
    return 0
  fi

  echo "> WARN: No supported terminal emulator found; running in background in this shell."
  bash -lc "$cmd" &
}

function ts_pipe() {
  # Prefer `ts` (moreutils). Fallback to awk timestamping.
  if command -v ts >/dev/null 2>&1; then
    ts "[$1]"
  else
    awk -v fmt="$1" '{ print strftime("[" fmt "]"), $0 }'
  fi
}

function kill_serial_readers() {
  # Best-effort cleanup of existing readers that may hold the port open.
  # We intentionally scope to the specific port string to avoid killing unrelated sessions.
  local port="$1"
  if [[ -z "$port" ]]; then
    return 0
  fi

  # socat
  if command -v pkill >/dev/null 2>&1; then
    pkill -f "socat .*FILE:${port//\//\\/}" >/dev/null 2>&1 || true
    pkill -f "socat .*${port//\//\\/}" >/dev/null 2>&1 || true

    # picocom (direct or wrapped with script)
    pkill -f "picocom .* ${port//\//\\/}" >/dev/null 2>&1 || true
    pkill -f "script .*picocom .* ${port//\//\\/}" >/dev/null 2>&1 || true
  fi
}

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
    
    nrfjprog --com | while read -r sn port vcom; do
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
    if [[ $IS_MAC -eq 1 ]]; then
      PORT_GLOB="/dev/tty.usbmodem*"
    else
      PORT_GLOB="/dev/ttyACM*"
    fi
    for port in $PORT_GLOB; do
      [[ -e "$port" ]] || continue
      if [[ $IS_MAC -eq 1 ]]; then
        sn="UNKNOWN"
      else
        sn=`udevadm info -q property -a -p $(udevadm info -q path -n $port) | grep serial | grep -oP '(?<=").*(?=")' | grep "0006" | cut -c 4-`
      fi
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
  HAVE_SOCAT=0
  HAVE_PICOCOM=0
  if command -v socat >/dev/null 2>&1; then HAVE_SOCAT=1; fi
  if command -v picocom >/dev/null 2>&1; then HAVE_PICOCOM=1; fi
  if [[ $HAVE_SOCAT -eq 0 && $HAVE_PICOCOM -eq 0 ]]; then
    echo "> ERROR: -t requires either socat or picocom."
    if [[ $IS_MAC -eq 1 ]]; then
      echo ">        Install with: brew install socat   (recommended)"
      echo ">        Or:           brew install picocom"
    fi
    exit 2
  fi
  len=`expr ${#sn_arr[@]} - 1`
  if [ -n "${LOGS+x}" ]; then
    mkdir -p "$LOGS"
  else
    mkdir -p ~/logs
  fi
  if [ -n "${CLEAN+x}" ]; then
    if [ -n "${LOGS+x}" ]; then
      if [ -d "$LOGS" ]; then rm -Rf "$LOGS"; fi
      mkdir -p "$LOGS"
    else
      if [ -d ~/logs ]; then rm -Rf ~/logs; fi
      mkdir -p ~/logs
    fi
  fi
  nrfjprog --com | while read -r sn port vcom; do
    kill_serial_readers "$port"
    for i in $(seq 0 $len); do
      if [ ${sn_arr[$i]} == $sn ]; then
        id=${id_arr[$i]}
        mac=${mac_arr[$i]}
      fi
    done
    echo "  - Port: $port | Node ID: $id | MAC Addr: $mac | JLink SN: $sn"
    if [ -n "${LOGS+x}" ]; then
      if [[ $HAVE_SOCAT -eq 1 ]]; then
        if [[ $IS_MAC -eq 1 ]]; then
          SOCAT_OPTS="raw,echo=0,nonblock,ispeed=$baud,ospeed=$baud"
        else
          SOCAT_OPTS="raw,echo=0,nonblock,b$baud"
        fi
        if command -v ts >/dev/null 2>&1; then
          CMD="socat \"FILE:$port,$SOCAT_OPTS\" STDOUT | ts '[%Y-%m-%d %H:%M:%S]' | tee \"$LOGS/log_$id.txt\""
        else
          echo "> WARN: ts not found; output will not be timestamped. (Install: brew install moreutils)"
          CMD="socat \"FILE:$port,$SOCAT_OPTS\" STDOUT | tee \"$LOGS/log_$id.txt\""
        fi
      else
        echo "> WARN: socat not found; using picocom (logs will not be timestamped)."
        CMD="script -q \"$LOGS/log_$id.txt\" picocom -fh -b $baud -c --imap lfcrlf $port"
      fi
    else
      if [[ $HAVE_SOCAT -eq 1 ]]; then
        if [[ $IS_MAC -eq 1 ]]; then
          SOCAT_OPTS="raw,echo=0,nonblock,ispeed=$baud,ospeed=$baud"
        else
          SOCAT_OPTS="raw,echo=0,nonblock,b$baud"
        fi
        if command -v ts >/dev/null 2>&1; then
          CMD="socat \"FILE:$port,$SOCAT_OPTS\" STDOUT | ts '[%Y-%m-%d %H:%M:%S]'"
        else
          echo "> WARN: ts not found; output will not be timestamped. (Install: brew install moreutils)"
          CMD="socat \"FILE:$port,$SOCAT_OPTS\" STDOUT"
        fi
      else
        CMD="picocom -fh -b $baud -c --imap lfcrlf $port"
      fi
    fi
    spawn_terminal "$CMD" "nrf-$id"
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
  if [[ $IS_MAC -eq 1 ]]; then
    for port in /dev/tty.usbmodem*; do
      [[ -e "$port" ]] || continue
      acm_arr+=($port)
    done
  else
    for port in /dev/ttyACM*; do
      [[ -e "$port" ]] || continue
      acm_arr+=($port)
    done
  fi
  # get list of nrfjrpog ports
  njp_arr=()
  while read -r sn port vcom; do
    njp_arr+=($port)
  done <<< "$(nrfjprog --com)"

  usb_ports=($(arraydiff acm_arr[@] njp_arr[@]))

  for port in "${usb_ports[@]}"; do
  echo "  - Port: $port"
    # CMD="socat $port,b$baud,raw,echo=0,nonblock STDOUT | ts [%Y-%m-%d\ %H:%M:%.S]"
    CMD="picocom -fh -b $baud -c --imap lfcrlf $port"
    spawn_terminal "$CMD" "usb-$port"
  done
  wait
fi
