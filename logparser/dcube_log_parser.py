import re
import csv
import pandas as pd
import sys
import os
import argparse
from Plotting import DataPlotter
from fft_calculator import FFTCalculator
from statistics import StatsCalculator
import time

class DcubeLogParser:

    def __init__(self, bv, phy_lyr, len) -> None:
        self.main_dp = ''
        self.log_file = ''
        self.csv_file = ''
        self.log_dir = ''
        self.csv_file_dir = ''
        self.is_bv = bv
        self.tempwise = False
        self.physical_layer = phy_lyr
        self.pkt_len = len

    def set_tempwise(self, tempwise) -> None:
        """
        This function sets the tempwise flag
        :param tempwise: tempwise flag's value
        :return: None
        """
        self.tempwise = tempwise


    def parse_logs_bv(self) -> None:
        """
        This function parses the log file when bit voting is enabled and stores the data in a CSV file
        :return: None
        """

        # Regular expression patterns for extracting the required fields
        log_pattern = re.compile(r'(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d+)\|EP:(\d+),.*N_RX:(\d+),N_ERR_PKTS:(\d+),BV_CNT:(\d+),BV_SCS_FLAG:(-?\d+),ERRS:{(.*?)},SLTS:(\w+)')
        date_pattern = '%Y-%m-%d %H:%M:%S.%f'

        # Initialize lists to store the extracted data
        data = []

        # Open the log file and process each line
        with open(self.log_file, 'r') as f:
            for line in f:
                match = log_pattern.match(line)
                if match:
                    timestamp = match.group(1)
                    epoch = int(match.group(2))
                    n_rx = int(match.group(3))
                    n_err_pkts = int(match.group(4))
                    bv_count = int(match.group(5))
                    bv_scs_flag = int(match.group(6))
                    errs = '{'+match.group(7)+'}'
                    slts = match.group(8)
                    # if epoch >= 60:
                    data.append([timestamp, epoch, n_rx, n_err_pkts, bv_count, bv_scs_flag, errs, slts])
                        # print([timestamp, epoch, n_rx, n_err_pkts, bv_count, bv_scs_flag, errs, slts])
                        # sys.exit(1)

        # Write the extracted data to a CSV file
        with open(self.csv_file, 'w', newline='') as f:
            csv_writer = csv.writer(f)
            csv_writer.writerow(["Timestamp", "ROUND", "N_RX", "N_ERR_PKTS", "BV_COUNT", "BV_SCS_FLAG", "ERRS", "SLOTS"])
            csv_writer.writerows(data)

        print(f"Parsed logs have been written to {self.csv_file}")
    
    def parse_logs_bv_tempwise(self, start_temp, end_temp, temp_increase_interval, time_increase_interval) -> None:
        """
        This function parses the log file when bit voting is enabled and stores the data in a CSV file
        :return: None
        """

        # Regular expression patterns for extracting the required fields
        # log_pattern = re.compile(r'(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d+)\|EP:(\d+),.*N_RX:(\d+),N_ERR_PKTS:(\d+),BV_CNT:(\d+),BV_SCS_FLAG:(-?\d+),ERRS:{(.*?)},SLTS:(\w+)')
        log_pattern = re.compile(r'(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}\.\d+)\|EP:(\d+),.*N_RX:(\d+),N_ERR_PKTS:(\d+),BV_CNT:(\d+),BV_SCS_FLAG:(-?\d+),ERRS:{(.*?)},SLTS:(.*?)$')
        date_pattern = '%Y-%m-%d %H:%M:%S.%f'
        found_first_timestamp = False
        # Initialize lists to store the extracted data
        data = []

        # Open the log file and process each line
        with open(self.log_file, 'r') as f:
            for line in f:
                match = log_pattern.match(line)
                if match:
                    timestamp = pd.to_datetime(match.group(1))
                    epoch = int(match.group(2))
                    n_rx = int(match.group(3))
                    n_err_pkts = int(match.group(4))
                    bv_count = int(match.group(5))
                    bv_scs_flag = int(match.group(6))
                    errs = '{'+match.group(7)+'}'
                    slts = match.group(8)
                    # if epoch >= 60:
                    data.append([timestamp, epoch, n_rx, n_err_pkts, bv_count, bv_scs_flag, errs, slts])
                        # print([timestamp, epoch, n_rx, n_err_pkts, bv_count, bv_scs_flag, errs, slts])
                        # sys.exit(1)
                    if not found_first_timestamp:
                        start_time = timestamp
                        found_first_timestamp = True
                        end_time = start_time + pd.Timedelta(minutes=time_increase_interval)
                        current_temp = start_temp
                        # print(data)
                        # sys.exit(1)
                    
                    if timestamp >= end_time:
                        start_time = timestamp
                        end_time = start_time + pd.Timedelta(minutes=time_increase_interval)
                        csv_filename = self.csv_file_dir+"/output_"+self.log_file.split("/")[1].split(".")[0]+"_"+str(current_temp)+"C.csv"
                        current_temp += temp_increase_interval
                        
                        if current_temp > end_temp:
                            current_temp = start_temp
                        # print(len((data)))
                        # Write the extracted data to a CSV file
                        with open(csv_filename, 'w', newline='') as f:
                            csv_writer = csv.writer(f)
                            csv_writer.writerow(["Timestamp", "ROUND", "N_RX", "N_ERR_PKTS", "BV_COUNT", "BV_SCS_FLAG", "ERRS", "SLOTS"])
                            csv_writer.writerows(data)
                        # print('before resetting: ', data)
                        data = []
                        # print('after resetting: ', data)
                        print(f"Parsed logs have been written to {csv_filename}")
        if len(data) > 0:
            csv_filename = self.csv_file_dir+"/output_"+self.log_file.split("/")[1].split(".")[0]+"_"+str(current_temp)+"C.csv"
            # print(len((data)))
            # Write the extracted data to a CSV file
            with open(csv_filename, 'w', newline='') as f:
                csv_writer = csv.writer(f)
                csv_writer.writerow(["Timestamp", "ROUND", "N_RX", "N_ERR_PKTS", "BV_COUNT", "BV_SCS_FLAG", "ERRS", "SLOTS"])
                csv_writer.writerows(data)
            # print('before resetting: ', data)
            data = []
            # print('after resetting: ', data)
            print(f"Parsed logs have been written to {csv_filename}")

    def parse_logs_no_bv(self) -> None:
        """
        This function parses the log file when bit voting is disabled and stores the data in a CSV file
        :return: None
        """

        # Define the CSV header
        csv_header = ['TIMESTAMP','ROUND', 'N_RX', 'N_ERR_PKTS','ERRS', 'SLOTS']

        parsed_logs = []

        with open(self.log_file, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                timestamp, data = parts[0], parts[1]
                data_dict = {}
                data_dict['TIMESTAMP'] = timestamp
                data = data.split(",")
                # print(len(data))
                if len(data) == 5:
                    # print('data: ',data)
                    # if int(data[0].split(":")[1]) >= 60:
                    for entry in data:
                        key, value = entry.split(':', 1)
                        if key == 'SLTS':
                            key = 'SLOTS'
                        if key == 'EP':
                            key = 'ROUND'
                        data_dict[key] = value
                    # print(data_dict)
                    parsed_logs.append(data_dict)


        with open(self.csv_file, 'w', newline='') as csvfile:
            writer = csv.DictWriter(csvfile, fieldnames=csv_header)
            writer.writeheader()
            writer.writerows(parsed_logs)

        print(f'Parsed logs have been written to {self.csv_file}')
    
    def parse_logs_no_bv_tempwise(self, start_temp, end_temp, temp_increase_interval, time_increase_interval) -> None:
        """
        This function parses the log file when bit voting is disabled and stores the data in a CSV file temperature wise
        :return: None
        """

        # Define the CSV header
        csv_header = ['TIMESTAMP','ROUND', 'N_RX', 'N_ERR_PKTS','ERRS', 'SLOTS']

        parsed_logs = []
        found_first_timestamp = False
        with open(self.log_file, 'r') as f:
            for line in f:
                parts = line.strip().split('|')
                timestamp, data = parts[0], parts[1]
                data_dict = {}
                data_dict['TIMESTAMP'] = pd.to_datetime(timestamp)
                data = data.split(",")
                # print(len(data))
                if len(data) == 5:
                    # print('data: ',data)
                    # if int(data[0].split(":")[1]) >= 60:
                    if not found_first_timestamp:
                        start_time = data_dict['TIMESTAMP']
                        found_first_timestamp = True
                        end_time = start_time + pd.Timedelta(minutes=time_increase_interval)
                        current_temp = start_temp
                        # print(data)
                        # sys.exit(1)
                    if data_dict['TIMESTAMP'] >= end_time:
                        start_time = data_dict['TIMESTAMP']
                        end_time = start_time + pd.Timedelta(minutes=time_increase_interval)
                        csv_filename = self.csv_file_dir+"/output_"+self.log_file.split("/")[1].split(".")[0]+"_"+str(current_temp)+"C.csv"
                        current_temp += temp_increase_interval
                        
                        if current_temp > end_temp:
                            current_temp = start_temp
                        # print(len((data)))
                        # Write the extracted data to a CSV file
                        with open(csv_filename, 'w', newline='') as f:
                            writer = csv.DictWriter(f, fieldnames=csv_header)
                            writer.writeheader()
                            writer.writerows(parsed_logs)
                        # print('before resetting: ', parsed_logs)
                        parsed_logs = []
                        # print('after resetting: ', parsed_logs)
                        print(f"Parsed logs have been written to {csv_filename}")
                    
                    for entry in data:
                        key, value = entry.split(':', 1)
                        if key == 'SLTS':
                            key = 'SLOTS'
                        if key == 'EP':
                            key = 'ROUND'
                        data_dict[key] = value
                    # print(data_dict)
                    parsed_logs.append(data_dict)


        if len(parsed_logs) > 0:
            csv_filename = self.csv_file_dir+"/output_"+self.log_file.split("/")[1].split(".")[0]+"_"+str(current_temp)+"C.csv"
            with open(csv_filename, 'w', newline='') as csvfile:
                writer = csv.DictWriter(csvfile, fieldnames=csv_header)
                writer.writeheader()
                writer.writerows(parsed_logs)

            print(f'Parsed logs have been written to {self.csv_file}')

        

    
    def get_csv_file_path(self) -> str:
        """
        This function returns the CSV file path
        :return: CSV file path
        """

        return self.csv_file

    def get_main_dp(self) -> str:
        """
        This function returns the main directory path
        :return: main directory path
        """

        return self.main_dp

    def get_log_dir(self) -> str:
        """
        This function returns the log directory path
        :return: log directory path
        """

        return self.log_dir

    def calc_statistics_bv(self, stat_calc) -> None:
        """
        This function calculates the statistics from the CSV files
        :param stat_calc: StatsCalculator object
        :return: None   
        """
        print('---------PRR---------\n')
        stat_calc.calc_rx_prr()
        print('---------PDR---------\n')
        stat_calc.calc_rx_pdr()
        if self.is_bv:
            print('---------Bit Voting Effectiveness---------\n')
            stat_calc.calc_bit_voting_effectiveness()
            stat_calc.calc_error_corrected_error_positions()
        else:
            stat_calc.calc_error_positions()
        if self.tempwise:
            stat_calc.write_stats_tempwise()
        else:
            stat_calc.write_stats()

    def plot_errors(self, plotter, err_pos, corr_err_pos, temperature=None) -> None:
        """
        This function plots the error positions from the CSV files.
        :param error_positions: Dictionary of error positions and their frequencies
        :param corrected_error_positions: Dictionary of corrected error positions and their frequencies
        :param temperature: Temperature at which collected the data
        :return: None
        """
        
        plotter.set_physical_layer(args.physical_layer)
        plotter.get_all_file_paths()
        if not self.tempwise:
            plt_title = f'Bit Errors vs Bit Corrections per index:{self.physical_layer}, pkt_len:{self.pkt_len}'
        else:
            plt_title = f'Bit Errors vs Bit Corrections per index:{self.physical_layer}, pkt_len:{self.pkt_len}, temp:{temperature}'
        plotter.plot_error_positions(err_pos, plt_title, temperature,corr_err_pos)
        # time.sleep(10)

    def set_file_paths_bv(self, log_file_path) -> None:
        """
        This function sets the file paths when bit voting is enabled
        :param log_file_path: path of the log file
        :return: None
        """

        self.main_dp = "./Dcube_Logs/With_BV/"+args.physical_layer
        if not os.path.exists(self.main_dp):
            os.makedirs(self.main_dp)
        grph = self.main_dp+"/Graphs"
        if not os.path.exists(grph):
            os.makedirs(grph)
        self.log_file = "./Dcube_Logs/With_BV/"+args.physical_layer+"/"+log_file_path  # Replace with the path to your log file
        self.log_dir = "./Dcube_Logs/With_BV/"+args.physical_layer+"/"+log_file_path.split("/")[0]
        self.csv_file_dir = self.log_dir+"/CSVFiles"
        if not os.path.exists(self.csv_file_dir):
            os.makedirs(self.csv_file_dir)
        self.csv_file = self.csv_file_dir+"/output_"+log_file_path.split("/")[1].split(".")[0]+".csv"

    def set_file_paths_no_bv(self, log_file_path) -> None:
        """
        This function sets the file paths when bit voting is disabled
        :param log_file_path: path of the log file
        :return: None
        """
        
        self.main_dp = "./Dcube_Logs/No_BV/"+args.physical_layer
        if not os.path.exists(self.main_dp):
            os.makedirs(self.main_dp)
        self.log_file = "./Dcube_Logs/No_BV/"+args.physical_layer+"/"+log_file_path  # Replace with the path to your log file
        self.log_dir = "./Dcube_Logs/No_BV/"+args.physical_layer+"/"+log_file_path.split("/")[0]
        grph = self.main_dp+"/Graphs"
        if not os.path.exists(grph):
            os.makedirs(grph)
        self.csv_file_dir = self.log_dir+"/CSVFiles"
        if not os.path.exists(self.csv_file_dir):
            os.makedirs(self.csv_file_dir)
        self.csv_file = self.csv_file_dir+"/output_"+log_file_path.split("/")[1].split(".")[0]+".csv"        

if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('-path', '--log_file_path', type=str, help='Path to the log file', required=True)
    arg_parser.add_argument('-bv', '--bv', type=int, help='Bit voting enabled or not', required=True)
    arg_parser.add_argument('-phy','--physical_layer', type=str, help='Physical layer used', required=True)
    arg_parser.add_argument('-len','--packet_length', type=int, help='Packet length used', required=True)
    arg_parser.add_argument('-tmp','--temperature_wise', type=int, help='Temperature wise', required=False)
    arg_parser.add_argument('-start_tmp','--start_temperature', type=int, help='Start temperature', required=False)
    arg_parser.add_argument('-end_tmp','--end_temperature', type=int, help='End temperature', required=False)
    arg_parser.add_argument('-temp_int','--temperature_increase_interval', type=int, help='Temperature increase interval', required=False)
    arg_parser.add_argument('-time_int','--time_increase_interval', type=int, help='Time increase interval', required=False)

    args = arg_parser.parse_args()

    required_args = ['log_file_path', 'bv', 'physical_layer', 'packet_length']
    missing_args = [arg for arg in required_args if getattr(args, arg) is None]

    all_arguments_provided = all(getattr(args, arg) is not None for arg in vars(args))

    if missing_args:
        print('Please provide all the required arguments...')
        arg_parser.print_help()
        sys.exit(1)
    
    if getattr(args, 'temperature_wise') is not None:
        if getattr(args, 'start_temperature') is None or getattr(args, 'end_temperature') is None or getattr(args, 'temperature_increase_interval') is None or getattr(args, 'time_increase_interval') is None:
            print('Please provide all the required arguments...')
            arg_parser.print_help()
            sys.exit(1)

    log_file_path = args.log_file_path
    print(args.bv)
    parser = DcubeLogParser(args.bv, args.physical_layer, args.packet_length)
    if getattr(args, 'temperature_wise') is not None:
        parser.set_tempwise(True)
    print(log_file_path.split("/")[0])

    
    if args.bv == 1:
        parser.set_file_paths_bv(log_file_path)
        if parser.tempwise:
            parser.parse_logs_bv_tempwise(args.start_temperature, args.end_temperature, args.temperature_increase_interval, args.time_increase_interval)
        else:
            parser.parse_logs_bv()
        stat_calc = StatsCalculator(parser.get_main_dp(), parser.get_log_dir(), args.bv, parser.tempwise)
        parser.calc_statistics_bv(stat_calc)
        plotter = DataPlotter(parser.get_main_dp(),parser.get_log_dir(),args.packet_length, args.bv)
        fft_calculator = FFTCalculator(parser.get_main_dp(), args.physical_layer, args.bv)
        print('--------- Plotting Error Positions ---------\n')
        if parser.tempwise:
            error_tmpwise_dict = stat_calc.get_error_positions_tempwise_dictionary()
            corrected_tmpwise_dict = stat_calc.get_corrected_error_positions_tempwise_dictionary()

            # print(error_tmpwise_dict.keys())
            # print(corrected_tmpwise_dict.keys())

            for err_temp, corr_err_temp in zip(error_tmpwise_dict.keys(), corrected_tmpwise_dict.keys()):
                print('--------- Plotting Error Positions for temperature: ', err_temp, corr_err_temp,' ---------\n')
                parser.plot_errors(plotter, error_tmpwise_dict[err_temp], corrected_tmpwise_dict[corr_err_temp], err_temp)
            
            print('---------FFT---------\n')
            for err_temp in error_tmpwise_dict.keys():
                print('--------- Plotting FFT for temperature: ', err_temp, ' ---------\n')
                sample_freq, pwr, cutoff_freq = fft_calculator.fft_calculator(error_tmpwise_dict[err_temp], err_temp)
                plotter.plot_fft(sample_freq, pwr, cutoff_freq, err_temp)
        else:
            parser.plot_errors(plotter, stat_calc.get_error_positions_dictionary(), stat_calc.get_corrected_error_positions_dictionary())
            print('---------FFT---------\n')
            sample_freq, pwr, cutoff_freq = fft_calculator.fft_calculator(stat_calc.get_error_positions_dictionary())
            plotter.plot_fft(sample_freq, pwr, cutoff_freq)


    else:
        parser.set_file_paths_no_bv(log_file_path)
        if parser.tempwise:
            parser.parse_logs_no_bv_tempwise(int(args.start_temperature), int(args.end_temperature), int(args.temperature_increase_interval), int(args.time_increase_interval))
        else:
            parser.parse_logs_no_bv()
        stat_calc = StatsCalculator(parser.get_main_dp(), parser.get_log_dir(), args.bv, parser.tempwise)
        parser.calc_statistics_bv(stat_calc)
        plotter = DataPlotter(parser.get_main_dp(),parser.get_log_dir(),args.packet_length, args.bv)
        fft_calculator = FFTCalculator(parser.get_main_dp(), args.physical_layer, args.bv)
        print('--------- Plotting Error Positions ---------\n')

        if parser.tempwise:
            error_tmpwise_dict = stat_calc.get_error_positions_tempwise_dictionary()
            for err_temp in error_tmpwise_dict.keys():
                print('--------- Plotting Error Positions for temperature: ', err_temp, ' ---------\n')
                parser.plot_errors(plotter, error_tmpwise_dict[err_temp],{}, err_temp)
            
            print('---------FFT---------\n')
            for err_temp in error_tmpwise_dict.keys():
                print('--------- Plotting FFT for temperature: ', err_temp, ' ---------\n')
                sample_freq, pwr, cutoff_freq = fft_calculator.fft_calculator(error_tmpwise_dict[err_temp], err_temp)
                plotter.plot_fft(sample_freq, pwr, cutoff_freq, err_temp)
        else:    
            parser.plot_errors(plotter, stat_calc.get_error_positions_dictionary(), stat_calc.get_corrected_error_positions_dictionary())
        
            print('---------FFT---------\n')
            sample_freq, pwr, cutoff_freq = fft_calculator.fft_calculator(stat_calc.get_error_positions_dictionary())
            plotter.plot_fft(sample_freq, pwr, cutoff_freq)