# declare -a phy_arr=('BLE_2M' 'BLE_1M' 'BLE_500K' 'BLE_125K' 'IEEE')
# declare -a jam_arr=(0 1 3)
declare -a phy_arr=('BLE_1M')
declare -a jam_arr=(0)

LOGGING=1
GPIO=0

# Use AD site (default is Graz)
# SITE="http://172.31.107.18"

if [[ $SITE == "http://172.31.107.18" ]]; then
  KEY="adadmin"
  if [[ $KEY == "adadmin" ]]; then
    ADMIN_PROTO=1
    DIS_PROTO=1
    COL_PROTO=5
  fi
else
  # KEY="ssrc"
  KEY="michael"
  if [[ $KEY == "ssrc" ]]; then
    DIS_PROTO=6804
    COL_PROTO=6825
  else
    DIS_PROTO=6808
    COL_PROTO=6846
  fi
fi

# --------------------------------------------------------------------------- #
# IPv6
DATA=8
DUR=120
PERIOD=1

for j in "${jam_arr[@]}"; do
  for p in "${phy_arr[@]}"; do
    ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $DIS_PROTO --dur=$DUR --j=${j} --layout=4 --data=$DATA -d "IPv6" PHY=PHY_"${p}" PWR=ZerodBm PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO IPV6=1 NTA=6 NTX=3 RTX=0 WITH_BORDER_ROUTER=1 ISN=1 PROTO=OSF_PROTO_STA DCUBE=1 -sha -pri
  done
done

# --------------------------------------------------------------------------- #
# Dissemination
# DUR=120
# PERIOD=1000

# for j in "${jam_arr[@]}"; do
#   for p in "${phy_arr[@]}"; do
#     ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $DIS_PROTO --dur=$DUR --j=${j} --layout=4 --data=$DATA -d "DISSEMINATION" PHY=PHY_"${p}" PWR=ZerodBm PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX PROTO=OSF_PROTO_BCAST DCUBE=1 -sha
#   done
# done

# --------------------------------------------------------------------------- #
# Collection
# DUR=1200
# LAYOUT=4
# PERIOD=1000
# NTA=12

# for j in "${jam_arr[@]}"; do
#   for p in "${phy_arr[@]}"; do
#     ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --j=${j} --layout=$LAYOUT --data=$DATA -d "NOTOG" PHY=PHY_"${p}" PWR=ZerodBm PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=0 TOG=0 ALWAYS_ACK=0 -sha
#   done
#   for p in "${phy_arr[@]}"; do
#     ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --j=${j} --layout=$LAYOUT --data=$DATA -d "ACK" PHY=PHY_"${p}" PWR=ZerodBm PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=0 TOG=1 ALWAYS_ACK=1 -sha
#   done
# done

# # --------------------------------------------------------------------------- #
# # Collection - MPHY
# for j in "${jam_arr[@]}"; do
# ./dcube.sh -POST KEY=$KEY TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -p $COL_PROTO --dur=$DUR --j=${j} --layout=$LAYOUT --data=$DATA -d "MPHY_125_IEEE_500" PWR=ZerodBm PERIOD=$PERIOD CHN=1 LOGGING=$LOGGING GPIO=$GPIO NTX=$NTX NSLOTS=$NSLOTS NTA=$NTA PROTO=OSF_PROTO_STA DCUBE=1 EMPTY=0 TOG=1 ALWAYS_ACK=1 -sha
# done

# # ----------------------------------------------------------------------------- #
# # E2E - Demo
# DUR=300
# ./dcube.sh -POST KEY=$KEY SITE=$SITE TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -d "E2E DEMO TEST" -p $E2E_PROTO --dur=$DUR --j=0 --layout=1 -sha TESTBED=dcube DEPLOYMENT=dcube PERIOD=1 CHN=1 LOGGING=0 GPIO=0 LEDS=1 PWR=Pos8dBm PROTO=OSF_PROTO_STA NTA=6 NTX=3 IPV6=1 WITH_WEBSERVER=1 WITH_BORDER_ROUTER=1 BR=51,80 FEM=0 CRYPTO=0 PHY=PHY_BLE_2M RTX=1 -pri 
# ./dcube.sh -POST KEY=$KEY SITE=$SITE TARGET=nrf52840 BOARD=dk -e osf -n "OSF" -d "E2E DEMO TEST CAT" -p 5 --dur=$DUR --j=0 --layout=0 -sha TESTBED=dcube DEPLOYMENT=dcube PERIOD=1 CHN=1 LOGGING=0 GPIO=0 LEDS=1 PWR=ZerodBm PROTO=OSF_PROTO_STA NTA=6 NTX=1 IPV6=1 WITH_WEBSERVER=1 WITH_BORDER_ROUTER=1 BR=51,80 FEM=0 CRYPTO=0 PHY=PHY_BLE_2M RTX=1 USB=1 -pri 