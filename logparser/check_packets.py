import sys
import re
import subprocess
class checkPackets:

    def __init__(self) -> None:
        self.start_pkt = ''
        self.rx_ok_pkt = ''

    def check_pkts(self, file_path):
        input_process = subprocess.Popen(['tail', '-f', file_path], stdout=subprocess.PIPE)
        ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
        
        # Read and write the continuously updating input file
        while True:
            line = input_process.stdout.readline().decode().strip()
            line = ansi_escape.sub('', line)
                
            print(line)
            
            if 'start' in line:
                hex_values = re.findall(r'(?<=\s)[0-9a-fA-F]{2}(?=\s|$)', line)
                self.start_pkt = ''.join(hex_values[5:])
                # print(hex_values)
            if 'rx_ok' in line:
                hex_values = re.findall(r'(?<=\s)[0-9a-fA-F]{2}(?=\s|$)', line)
                self.rx_ok_pkt = ''.join(hex_values[5:])
                if len(self.start_pkt) > 0 and len(self.rx_ok_pkt) > 0 and self.start_pkt != self.rx_ok_pkt:
                    print('not equal')
                    break                
            # sys.exit()
            print('-----------------------------')


if __name__ == '__main__':
    ck = checkPackets()
    ck.check_pkts('./putty_dst.log')