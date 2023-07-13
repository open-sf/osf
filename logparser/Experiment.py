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

class Experiment:
    def __init__(self, pl, ed, source, destination, power) -> None:
        self.physical_layer = pl #Name of the physical layer used for the experiment
        self.experiment_duration = ed #Duration of the experiment in minutes
        self.packet_length = 116 if self.physical_layer == "PHY_IEEE" else 246 #Length of the payload used for the experiment
        self.dst_port = '' #Serial port of the JLink device (Destination)
        self.src = source #Source node ID
        self.dst = destination #Destination node ID
        self.picocom_output_file = '' #Path of the picocom output file
        self.power = power #Power of the JLink device
        self.logs_dir = '' #Path of the logs directory
    

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

        self.picocom_output_file = f'{self.logs_dir}/dst_log_{self.physical_layer}.log'
        if os.path.exists(self.picocom_output_file):
            os.remove(self.picocom_output_file)
    
    
    def compile_experiment(self):
        """
        This function compiles the experiment using make command which compiles the contiki application 
        and flashes it to the JLink device
        :return: True if the compilation is successful, False otherwise
        """

        directory = '/home/burhanuddin/Desktop/osf-private/examples/osf'
        
        make_command = f'make clean TARGET=nrf52840 && make -j16 node.upload-all TARGET=nrf52840 BOARD=dk DEPLOYMENT=nulltb TESTBED=nulltb SRC={self.src} DST={self.dst} PERIOD=1000 CHN=0 LOGGING=1 GPIO=1 LEDS=1 NTX=6 NSLOTS=6 PWR={self.power} PROTO=OSF_PROTO_BCAST PHY={self.physical_layer} BV=1 LENGTH={self.packet_length}'
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
        
    def start_picocom(self):
        """
        This function starts picocom to read the serial output from the JLink device
        :return: subprocess object if picocom is started successfully, False otherwise
        """

        self.set_picocom_output_file_path()
        pico_cmd = f'picocom -fh -b 115200 --imap lfcrlf {self.dst_port}'
        
        with open(self.picocom_output_file, 'w') as f:
            try:
                pico_process = subprocess.Popen(pico_cmd, shell=True, stdout=f, stderr=subprocess.STDOUT, preexec_fn=os.setsid)
                print('Picocom started...')
                return pico_process
            except subprocess.CalledProcessError as e:
                print('Failed to start picocom...')
                print(e.output)
                return False

    def run_timestamping(self, pico_process):
        """
        This function runs the timestamping script to add timestamps to the serial output
        :param pico_process: subprocess object of picocom
        :return: None
        """

        print('Running timestamping...')
        timestamp = AddTS(self.experiment_duration, self.logs_dir)
        try:
            return_code = timestamp.add_timestamp(self.picocom_output_file)
        except UnicodeDecodeError as e:
            print('Failed to run timestamping...')
            print(e.output)
            pyautogui.hotkey('ctrl', 'a')
            pyautogui.hotkey('ctrl', 'x')
            pico_process.wait()
            sys.exit(1)

        if return_code:
            print('Timestamping done...')
            try:
                # os.kill(pico_process.pid, signal.SIGTERM)
                # os.kill(pico_process.pid, signal.SIGINT)
                pyautogui.hotkey('ctrl', 'a')
                pyautogui.hotkey('ctrl', 'x')
                pico_process.wait()
            except subprocess.SubprocessError as e:
                print('Failed to kill picocom...')
                print(e.output)
            except OSError as e:
                print('Failed to kill picocom...')
                print(e.output)
    
    def make_csvs(self):
        """
        This function makes the CSVs from the logs
        :return: None
        """

        print('Making CSVs...')
        parser = Parser(self.logs_dir)
        parser.execute_csv_generation()

    def plot_data(self):
        """
        This function plots the data from the CSVs and calculates required metrics
        :return: None
        """

        print('Plotting data...')
        plotter = DataPlotter(self.logs_dir)
        plotter.set_physical_layer(self.physical_layer)
        plotter.get_all_file_paths()
        plotter.plot_error_positions()

    def get_statistics(self):

        print('Calculating statistics...')
        stats = StatsCalculator(self.logs_dir)
        stats.calc_avg_rx_before_correction()
        stats.calc_err_pkts_correct_pkt()
        stats.calc_rx_prr()


def main():
    """
    This is the main function which runs the experiment
    :return: None
    """

    arg_parser = argparse.ArgumentParser()
    # arg_parser.add_argument('-pl', '--physical_layer', type=str, help='Physical layer to use', required=True)
    arg_parser.add_argument('-ed', '--experiment_duration', type=int, help='Experiment duration in seconds', required=True)
    # arg_parser.add_argument('-pktlen', '--packet_length', type=int, help='Packet length in bytes', required=True)
    arg_parser.add_argument('-s', '--src', type=int, help='Source node', required=True)
    arg_parser.add_argument('-d', '--dst', type=int, help='Destination node', required=True)
    arg_parser.add_argument('-p', '--power', type=str, help='Power level', required=True)
    args = arg_parser.parse_args()

    if not(args.experiment_duration and args.src and args.dst and args.power):
        print('Please provide all the required arguments...')
        arg_parser.print_help()
        sys.exit(1)
    
    for phy in ['PHY_BLE_2M', 'PHY_BLE_1M', 'PHY_BLE_125K', 'PHY_BLE_500K','PHY_IEEE']:
        print(f'Running experiment for {phy}....')
        exp = Experiment(phy, args.experiment_duration, args.src, args.dst, args.power)
        exp.set_logs_dir()
        exp.get_destination_port()
        exp.compile_experiment()
        proc = exp.start_picocom()
        time.sleep(0.5)
        exp.run_timestamping(proc)
        exp.make_csvs()
        exp.plot_data()
        exp.get_statistics()
    print('Experiment done...')
    sys.exit(0)
if __name__ == '__main__':
    main()
        
