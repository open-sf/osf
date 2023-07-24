import datetime
import subprocess
from add_timestamp import AddTS
import os
import signal
import pyautogui
import time
from Plotting import DataPlotter
from gather_data import Parser
import argparse
import sys
from make_experiment_structure import ExperimentStructure
from statistics import StatsCalculator
import re
class Experiment:
    def __init__(self, pl, ed, pd_len,source, destination, power) -> None:
        self.physical_layer = pl #Name of the physical layer used for the experiment
        self.experiment_duration = ed #Duration of the experiment in minutes
        self.packet_length = pd_len #Length of the payload used for the experiment
        self.dst_port = '' #Serial port of the JLink device (Destination)
        self.src = source #Source node ID
        self.dst = destination #Destination node ID
        self.picocom_output_file = '' #Path of the picocom output file
        self.power = power #Power of the JLink device
        self.logs_dir = '' #Path of the logs directory
        self.is_bv = False #Boolean variable to check if the experiment has bit voting enabled or not 
        self.src_port = '' #Serial port of the JLink device (Source)
    

    def get_destination_port(self):
        """
        This function gets the serial destination port of the JLink device
        :return: True if the destination port is found, False otherwise
        """

        JLink_devices_id = {1:'683078767',2:'683292912',3:'683141591'}
        nrf_command = 'nrfjprog --com' 
        nrf_process_output = subprocess.run(nrf_command, capture_output=True, shell=True, text=True)

        if nrf_process_output.returncode == 0:
            print('Got the Destination port....')
            print(nrf_process_output.stdout)  
            for coms in nrf_process_output.stdout.splitlines():
                coms = coms.split()
                if coms[0] == JLink_devices_id[self.dst]:
                    self.dst_port = coms[1]
                    print('The Destination port is: ', self.dst_port)
                    return True
                if coms[0] == JLink_devices_id[self.src]:
                    self.src_port = coms[1]
                    print('The Source port is: ', self.src_port)
        else:
            print('Failed to get device ports....')
            print(f'Command: {nrf_process_output.args}')
            print(nrf_process_output.stderr)
        
        return False

    def set_logs_dir(self):
        """
        This function sets the logs directory for the experiment from the experiment file structure
        :return: None
        """
        exp_struct = ExperimentStructure()
        exp_struct.make_experiment_structure()
        self.logs_dir = exp_struct.get_physical_layer_directory(self.physical_layer)

    def set_picocom_output_file_path(self):
        """
        This function sets the picocom output file path for the experiment which contains the logs from the JLink device
        :return: None
        """
        if self.is_bv:
            self.picocom_output_file = f'{self.logs_dir}/dst_log_{self.physical_layer}.log'
        else:
            self.picocom_output_file = f'{self.logs_dir}/dst_log_{self.physical_layer}_no_bv.log'
        if os.path.exists(self.picocom_output_file):
            os.remove(self.picocom_output_file)
    
    
    def compile_experiment(self):
        """
        This function compiles the experiment using make command which compiles the contiki application 
        and flashes it to the JLink device
        :return: True if the compilation is successful, False otherwise
        """

        directory = '/home/burhanuddin/Desktop/osf-private/examples/osf'
        
        make_command = f'make clean TARGET=nrf52840 && make -j16 node.upload-all TARGET=nrf52840 BOARD=dk DEPLOYMENT=nulltb TESTBED=nulltb SRC={self.src} DST={self.dst} PERIOD=1000 CHN=0 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 PWR={self.power} PROTO=OSF_PROTO_BCAST PHY={self.physical_layer} BV={1 if self.is_bv == True else 0} LENGTH={self.packet_length}'
        print('Compiling experiment...')
        print(make_command)

        make_process_output = subprocess.run(make_command, capture_output=True, cwd=directory, shell=True, text=True)

        if make_process_output.returncode == 0:
            print('Compilation done...')    
            return True
        else:
            print('Failed to compile experiment...')
            print(f'Command: {make_process_output.args}')
            print(make_process_output.stderr)
            return False

    def run_picocom(self):
        source_command = f'picocom -fh -b 115200 --imap lfcrlf {self.src_port}'
        destination_command = f'picocom -fh -b 115200 --imap lfcrlf {self.dst_port}'
        ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
        source_process = subprocess.Popen(source_command.split(), stdin=subprocess.PIPE,stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        destination_process = subprocess.Popen(destination_command.split(), stdin=subprocess.PIPE,stdout=subprocess.PIPE, stderr=subprocess.STDOUT)

        timestamp = AddTS(self.experiment_duration, self.logs_dir)
        log_folder = timestamp.create_log_folder()
        self.picocom_output_file = log_folder+'/output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt'
        output_file = open(self.picocom_output_file, "a")
        packet_number = 0
        stop_value = 10
        # Read the output from both processes
        while True:
            # Read and process the output from the source process
            try:
                source_output = source_process.stdout.readline().decode().strip()
            except UnicodeDecodeError:
                pass
            try:
                if source_output and 'Packet_number' in source_output:
                    source_output = ansi_escape.sub('', source_output)
                    # Check if the packet number reaches the stop value
                    packet_number = int(source_output.split(':')[-1].strip())  # Assuming packet number is the first value
                    print('Source Packet_number: ', packet_number)
                    print('-----------------------------')
                    stop_value = 10  # Change this to your desired stop value
                    if packet_number >= stop_value:
                        # Terminate both processes if the stop value is reached
                        print(f'---Packet number {packet_number} reached the stop value {stop_value}---')
                        source_process.stdin.write(b'\x01\x18')
                        source_process.stdin.flush()
                        source_process.wait()
            except TypeError:
                pass
            
            try:
                # Read and process the output from the destination process
                destination_output = destination_process.stdout.readline().decode().strip()
                destination_output = ansi_escape.sub('', destination_output)
                if destination_output:
                    # Timestamp the destination output
                        to_be_written = '['+datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')+']'+" "+ destination_output
                        # print('-----------------------------')
                        print(to_be_written)
                        print('-----------------------------')
                        output_file.write(to_be_written + '\n')
                        output_file.flush()
                        if packet_number >= stop_value:
                            destination_process.stdin.write(b'\x01\x18')
                            destination_process.stdin.flush()
                            destination_process.wait()
                            break
            except UnicodeDecodeError:
                pass
            except TypeError:
                pass
        output_file.close()
        return log_folder
    
    def make_csvs(self, logs_dir):
        """
        This function makes the CSVs from the logs
        :return: None
        """

        print('Making CSVs...')
        parser = Parser(logs_dir, self.is_bv)
        parser.execute_csv_generation()

    def plot_data(self, logs_dir):
        """
        This function plots the data from the CSVs and calculates required metrics
        :return: None
        """

        print('Plotting data...')
        plotter = DataPlotter(self.logs_dir,logs_dir)
        plotter.set_physical_layer(self.physical_layer)
        plotter.get_all_file_paths()
        plotter.plot_error_positions()

    def get_statistics(self, logs_dir):
        """
        This function calculates the required statistics from
        the logs
        :return: None
        """
        
        print('Calculating statistics...')
        stats = StatsCalculator(self.logs_dir,logs_dir)

        if not self.is_bv:
            stats.calc_rx_prr_no_bv()
            # stats.calc_rx_pdr_no_bv()
        else:

            stats.calc_avg_rx_before_correction()
            stats.calc_err_pkts_correct_pkt()
            stats.calc_suc_fail_bv_count()
            stats.calc_rx_prr()
            stats.calc_rx_pdr()
            stats.calc_total_nrxs()

def main():
    """
    This is the main function which runs the experiment
    :return: None
    """

    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('-ed', '--experiment_duration', type=int, help='Experiment duration in seconds', required=True)
    arg_parser.add_argument('-pldlen', '--payload_length', type=int, help='Payload length in bytes', required=True)
    arg_parser.add_argument('-s', '--src', type=int, help='Source node ID', required=True)
    arg_parser.add_argument('-d', '--dst', type=int, help='Destination node ID', required=True)
    arg_parser.add_argument('-p', '--power', type=str, help='Transmit Power level of the nodes', required=True)
    arg_parser.add_argument('-bv', '--bitvoting', type=int, help='Enable Bit voting the error correction mechanism', required=True)
    args = arg_parser.parse_args()

    all_arguments_provided = all(getattr(args, arg) is not None for arg in vars(args))

    if not(all_arguments_provided):
        print('Please provide all the required arguments...')
        arg_parser.print_help()
        sys.exit(1)

    #for phy in ['PHY_BLE_2M','PHY_BLE_1M', 'PHY_BLE_125K', 'PHY_BLE_500K','PHY_IEEE']:
    for phy in ['PHY_BLE_2M','PHY_BLE_1M', 'PHY_BLE_125K', 'PHY_BLE_500K','PHY_IEEE']:
        print(f'Running experiment for {phy}....')
        exp = Experiment(phy, args.experiment_duration, args.payload_length, args.src, args.dst, args.power)
        if args.bitvoting == 1:
            exp.is_bv = True
        exp.set_logs_dir()
        exp.get_destination_port()
        exp.compile_experiment()
        txt_log_dir = exp.run_picocom()
        # txt_log_dir = exp.run_timestamping(proc)
        print('txt_log_dir', txt_log_dir)
        exp.make_csvs(txt_log_dir)
        if exp.is_bv == True: 
            exp.plot_data(txt_log_dir)
        exp.get_statistics(txt_log_dir)
    print('Experiment done...')
    sys.exit(0)
if __name__ == '__main__':
    main()
        
