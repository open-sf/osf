declare -a phy_arr=('BLE_2M' 'BLE_1M' 'BLE_500K' 'BLE_125K' 'IEEE')
declare -a jam_arr=(0 1 3)
declare -a pwr_arr=(ZerodBm)
declare -a data_arr=(8 64 118)
declare -a layout_arr=(3 4 6)


DATA=64
LOGGING=0
GPIO=0
NTX=6
NSLOTS=12
PWR=ZerodBm

KEY="michael"
# KEY="ssrc"

if [[ $KEY == "ssrc" ]]; then
  DIS_PROTO=6804
  COL_PROTO=6825
else
  DIS_PROTO=6808
  COL_PROTO=6846
fi

# --------------------------------------------------------------------------- #
# Dissemination
# DUR=600
# PERIOD=200
# LAYOUT=4
#
# for d in "${data_arr[@]}"; do
#   for j in "${jam_arr[@]}"; do
#     for p in "${phy_arr[@]}"; do
#       ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $DIS_PROTO --dur=$DUR --j=${j} --s=0 --layout=$LAYOUT --data=${d} -d "${d}B" LENGTH=${d} PHY=PHY_"${p}" PWR=$PWR PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS RNTX=0 PROTO=OSF_PROTO_BCAST DCUBE=1 -sha
#     done
#   done
# done


# --------------------------------------------------------------------------- #
# BACKOFF
# DUR=600
# LAYOUT=6
# PERIOD=1000
# NTA=12

# NSLOTS=24
# ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --period=5000 --j=0 --s=1 --layout=$LAYOUT --data=$DATA -d "PERIODIC_2M_24" LENGTH=$DATA PHY=PHY_BLE_2M PWR=$PWR PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=4 RNTX=0 -sha
# ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --period=5000 --j=0 --s=0 --layout=$LAYOUT --data=$DATA -d "PERIODIC_2M_RNTX" LENGTH=$DATA PHY=PHY_BLE_2M PWR=$PWR PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=4 RNTX=1 -sha -pri
# NSLOTS=12
# ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --period=5000 --j=0 --s=0 --layout=$LAYOUT --data=$DATA -d "PERIODIC_2M_12" LENGTH=$DATA PHY=PHY_BLE_2M PWR=$PWR PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=4 RNTX=0 -sha
# ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --period=5000 --j=0 --s=0 --layout=$LAYOUT --data=$DATA -d "PERIODIC_2M_BACKOFF" LENGTH=$DATA PHY=PHY_BLE_2M PWR=$PWR PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=4 BACKOFF=1 THRESHOLD=80 -sha -pri

# --------------------------------------------------------------------------- #
# Collection
# DUR=600
# PERIOD=1000
# NTA=12
# LAYOUT=3
#
# for d in "${data_arr[@]}"; do
#   for j in "${jam_arr[@]}"; do
#     for p in "${phy_arr[@]}"; do
#       ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --j=${j} --s=0 --layout=$LAYOUT --data=${d} -d "COL_${d}B" LENGTH=${d} PHY=PHY_${p} PWR=$PWR PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=4 -sha
#     done
#   done
# done

# --------------------------------------------------------------------------- #
# MPHY
DUR=600
PERIOD=1000
NTA=12

for p in "${pwr_arr[@]}"; do
  for l in "${layout_arr[@]}"; do
    ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --j=MB01 --s=0 --layout=${l} --data=$DATA -d "FINAL_SPHY_2M" LENGTH=$DATA PHY=PHY_BLE_2M PWR=${p} PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=0 -sha -pri
  done
  for l in "${layout_arr[@]}"; do
    ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --j=MB01 --s=0 --layout=${l} --data=$DATA -d "FINAL_SPHY_500K" LENGTH=$DATA PHY=PHY_BLE_500K PWR=${p} PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=0 -sha -pri
  done
  for l in "${layout_arr[@]}"; do
    ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --j=MB01 --s=0 --layout=${l} --data=$DATA -d "FINAL_MPHY" LENGTH=$DATA PWR=${p} PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=0 MPHY=1 -sha -pri
  done
  for l in "${layout_arr[@]}"; do
    ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --j=MB01 --s=0 --layout=${l} --data=$DATA -d "FINAL_MPHY_SERIAL" LENGTH=$DATA PWR=${p} PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=0 MPHY=1 -sha -pri
  done
done
