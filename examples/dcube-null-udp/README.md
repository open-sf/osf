TARGET=nrf52840 BOARD=dk TESTBED=nulltb DEPLOYMENT=nulltb WITH_BORDER_ROUTER=1 BR=<my_br_id>

sudo sysctl -w net.ipv6.conf.all.forwarding=1
sudo ip route add fd00::/64 via fd01::MY_BR_MAC dev tun0
sudo ../../tools/serial-io/tunslip6 -s /dev/ttyACM2 fd01::1/64 -B 115200
ping6 fd00::MY_PING_NODE_MAC
