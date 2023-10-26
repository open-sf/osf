import subprocess
import argparse
import sys
import os
def post_jobs(bv, fwd, src, dst, pktlen, full_phy, pwr):
    directory = '/home/burhanuddin/Desktop/osf/tools/dcube'
    if bv == 1:
        command = f"./dcube.sh -POST TARGET=nrf52840 -e osf -n BV -d 'bit voting {phy} power {pwr} jamming level none length {pktlen} actual dissemination  nulltb d{dst}f{fwd}s{src}' -p 6907 --s=1 -nopatch --dur=600 --data=64 --period=1000 -m DST={dst} FWD={fwd} SRC={src} TESTBED=nulltb PERIOD=1000 CHN=0 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 PWR={pwr} PROTO=OSF_PROTO_BCAST PHY={full_phy} BV={bv} LENGTH={pktlen - 9} DCUBE=1"
    else:
        command = f"./dcube.sh -POST TARGET=nrf52840 -e osf -n BV -d 'No bit voting {phy} power {pwr} jamming level none length {pktlen} actual dissemination nulltb d{dst}f{fwd}s{src}' -p 6907 --s=1 -nopatch --dur=600 --data=64 --period=1000 -m DST={dst} FWD={fwd} SRC={src} TESTBED=nulltb PERIOD=1000 CHN=0 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 PWR={pwr} PROTO=OSF_PROTO_BCAST PHY={full_phy} BV={bv} LENGTH={pktlen - 7} DCUBE=1"      
    
    print('------- Running Command ',command, ' -------\n')
    process_output = os.system(f'cd {directory} && {command}')

def post_jobs_templab(bv, fwd, src, dst, full_phy, pktlen, templab_file, pwr, message, duration):
    directory = '/home/burhanuddin/Desktop/osf/tools/dcube'
    
    if templab_file:
        if bv == 1:
            command = f"./dcube.sh -POST TARGET=nrf52840 -e osf -n BV -d 'bit voting {message} {phy}  power {pwr} jamming level none length {pktlen} actual dissemination templab nulltb d{dst}f{fwd}s{src}' -p 6907 --s=1 -nopatch --dur={duration} --data={pktlen - 9}  -m DST={dst} FWD={fwd} SRC={src} TESTBED=nulltb PERIOD=1000 CHN=0 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 PWR={pwr} PROTO=OSF_PROTO_BCAST PHY={full_phy} BV={bv} LENGTH={pktlen - 9} DCUBE=1 -templab {templab_file}"
        else:
            command = f"./dcube.sh -POST TARGET=nrf52840 -e osf -n BV -d 'no bit voting {message} {phy} power {pwr} jamming level none length {pktlen} actual dissemination templab nulltb d{dst}f{fwd}s{src}' -p 6907 --s=1 -nopatch --dur={duration} --data={pktlen - 7} -m DST={dst} FWD={fwd} SRC={src} TESTBED=nulltb PERIOD=1000 CHN=0 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 PWR={pwr} PROTO=OSF_PROTO_BCAST PHY={full_phy} BV={bv} LENGTH={pktlen - 7} DCUBE=1 -templab {templab_file}"

    print('------- Running Command ',command, ' -------\n')
    process_output = os.system(f'cd {directory} && {command}')
    


if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('-bv', '--bv', type=int, help='Bit voting enabled or not', required=True)
    arg_parser.add_argument('-fwd','--forwarder', type=int, help='Forwarder used', required=True)
    arg_parser.add_argument('-src','--source', type=int, help='Source used', required=True)
    arg_parser.add_argument('-dst','--destination', type=int, help='Destination used', required=True)
    arg_parser.add_argument('-pktlen','--packetlength', type=int, help='Packet length used', required=True)
    arg_parser.add_argument('-templab', help='Path to the Templab CSV file', default=None)
    arg_parser.add_argument('-pwr', '--power', type=str, help='Power used', required=True)
    arg_parser.add_argument('-m', '--message', type=str, help='Message used', required=True)
    arg_parser.add_argument('-dur', '--duration', type=int, help='Duration of the experiment', default=600)
    args = arg_parser.parse_args()

    required_args = ['bv', 'forwarder', 'source', 'destination', 'packetlength', 'power', 'message', 'duration']
    missing_args = [arg for arg in required_args if getattr(args, arg) is None]

    if missing_args:
        print(f'Please provide the following required arguments: {", ".join(missing_args)}')
        arg_parser.print_help()
        sys.exit(1)

    bv = args.bv
    fwd = args.forwarder
    src = args.source
    dst = args.destination
    for phy in ['2M','1M', '125K','500K']:
        full_phy = 'PHY_BLE_'+phy
        if args.templab:
            if not os.path.exists(args.templab):
                print("Error: Templab file does not exist")
                sys.exit(1)
            if not (120 <= fwd <= 127 and 120 <= src <= 127 and 120 <= dst <= 127):
                print("Error: fwd, src, and dst must be in the range of 120 to 127")
                sys.exit(1)
        
            post_jobs_templab(bv, fwd, src, dst, full_phy, args.packetlength, args.templab, args.power,args.message, args.duration)
        
        else:
            post_jobs(bv, fwd, src, dst, args.packetlength, full_phy, args.power, args.message)