import math
import os
import pandas as pd
from read_log_files import FileReader
import csv 

class StatsCalculator:
    """
    This class calculates the statistics from the CSV file created from the logs
    """

    def __init__(self, main_dp, log_dp, bv, tempwise) -> None:
        self.directory_path = main_dp # Path to the directory of the physical layer where the csv files are stored
        self.fr = FileReader(log_dp) # FileReader object
        self.file_paths = self.fr.get_csv_file_paths() # List of paths to the csv files
        self.corrected_packets = 0 # Number of packets corrected
        self.is_bv = bv # Bit voting enabled or not
        if not self.is_bv:
            self.dataframe = pd.DataFrame(columns=['PHY', 'PRR', 'PDR','Beating_Frequency'])
        else:
            self.dataframe = pd.DataFrame(columns=['PHY', 'PRR', 'PDR','Total_Corrections','Total_failed_corrections','Beating_Frequency'])
        self.stat_dict = {} # Dictionary to store the statistics
        self.stat_dict['PHY'] = self.directory_path.split('/')[-1]
        self.error_positions = {}
        self.corrected_error_positions = {}
        self.tempwise = tempwise
        if self.tempwise:
            self.error_positions_tempwise = {}
            self.corrected_error_positions_tempwise = {}

    def reset_stat_dict(self) -> None:
        """
        This function resets the stat_dict
        :return: None
        """

        self.stat_dict = {}

    def reset_dataframe(self) -> None:
        """
        This function resets the dataframe
        :return: None
        """

        if not self.is_bv:
            self.dataframe = pd.DataFrame(columns=['PHY', 'PRR', 'PDR','Beating_Frequency'])
        else:
            self.dataframe = pd.DataFrame(columns=['PHY', 'PRR', 'PDR','Total_Corrections','Total_failed_corrections','Beating_Frequency'])

    def write_stats(self) -> None:
        """
        This function writes the observations to a csv file
        :return: None
        """
        physical_layer = self.directory_path.split('/')[-1]
        if self.is_bv:
            file_path = self.directory_path + f'/statistics_{physical_layer}_bv.csv'
        else:
            file_path = self.directory_path + f'/statistics_{physical_layer}_no_bv.csv'

        print('------------Writing Calculated statistics to CSV file: ', file_path,'---------------------\n')
        new_df = pd.DataFrame([self.stat_dict])
        self.data_frame = pd.concat([self.dataframe, new_df], ignore_index=True)
        self.data_frame.to_csv(file_path, index=False)
    
    def write_stats_tempwise(self) -> None:
        """
        This function writes the observations to a csv file and there is a different file for every temperature
        :return: None
        """
        # physical_layer = self.directory_path.split('/')[-1]
        print(self.stat_dict)
        for keys, temperature_stat_dict in self.stat_dict.items():
            if keys == 'PHY':
                physical_layer = temperature_stat_dict
            else:
                if self.is_bv:
                    file_path = self.directory_path + f'/statistics_{keys}_{physical_layer}_bv.csv'
                else:
                    file_path = self.directory_path + f'/statistics_{keys}_{physical_layer}_no_bv.csv'

                print('------------Writing Calculated statistics to CSV file: ', file_path,'---------------------\n')
                new_df = pd.DataFrame([temperature_stat_dict])
                self.data_frame = pd.concat([self.dataframe, new_df], ignore_index=True)
                self.data_frame.to_csv(file_path, index=False)
                self.reset_dataframe()

    
    def calc_avg_rx_before_correction(self) -> None:
        """
        This function calculates the average number of receptions taken to
        successfully correct an error.
        :return: None
        """
        total_bv_count = 0
        num_oks = 0
        avg_bv_count = 0
        for filename in self.file_paths:
            print('Reading file: ', filename)
            data = pd.read_csv(filename)
            for (bv_count, ok) in zip(data['BV_COUNT'], data['BV_SCS_FLAG']):
                bv_count = int(bv_count)

                if ok == 1:
                    total_bv_count += bv_count
                    num_oks += 1
        print('\nThis statistics is with Bit voting enaled\n')
        if num_oks > 0:
            avg_bv_count = total_bv_count/num_oks
            print(f'Avg. Rx before correction: {avg_bv_count}\n')
            
            self.stat_dict['Avg_Rx_before_correction'] = avg_bv_count
        else:
            print('No successful corrections\n')
            self.stat_dict['Avg_Rx_before_correction'] = 0
    
    def calc_bit_voting_effectiveness(self) -> None:
        """
        This function calculates the effectiveness of bit voting by calculating total number of packet corrected
        based on BV_SUCCESS_FLAG
        :return: None
        """

        tot_err_pkts = 0 
        tot_correct_pkts = 0
        tot_correct_failed_pkts = 0
        for filename in self.file_paths:
            print('Reading file Bit Voting Effectiveness: ', filename)
            data = pd.read_csv(filename)
            tot_err_pkts = 0 
            tot_correct_pkts = 0
            tot_correct_failed_pkts = 0
            for (err, ok) in zip(data['N_ERR_PKTS'], data['BV_SCS_FLAG']):
                err = int(err)
                ok = int(ok)
                tot_err_pkts += err

                if ok == 1:
                    tot_correct_pkts += err
                elif ok == 0:
                    tot_correct_failed_pkts += err
            self.corrected_packets = tot_correct_pkts
            print(f'Total error packets: {tot_err_pkts}\n')
            print(f'Total packets corrected: {tot_correct_pkts}\n')
            print(f'Total packets correction failed: {tot_correct_failed_pkts}\n')
            if self.tempwise:
                temp = filename.split('/')[-1].split('_')[-1].split('.')[0]
                if temp not in self.stat_dict:
                    self.stat_dict[temp] = {}
                    self.stat_dict[temp]['Total_Corrections'] = tot_correct_pkts
                    self.stat_dict[temp]['Total_failed_corrections'] = tot_correct_failed_pkts
                else:
                    self.stat_dict[temp]['Total_Corrections'] = tot_correct_pkts
                    self.stat_dict[temp]['Total_failed_corrections'] = tot_correct_failed_pkts
            else:
                self.stat_dict['Total_Corrections'] = tot_correct_pkts
                self.stat_dict['Total_failed_corrections'] = tot_correct_failed_pkts

            print(f'Percentage of packets corrected: {(tot_correct_pkts/max(1,(tot_correct_failed_pkts + tot_correct_pkts)))*100}\n')


    def calc_rx_prr(self) -> None:
        """
        This function calculates the packet reception rate
        -PRR = total number of correct reception / total number of packets sent
        -Total number of Correct Receptions per round is N_RX_PKTS - N_ERR_PKTS
        :return: None
        """
        print('File paths: ',self.file_paths)
        tot_correct_rx = 0
        tot_failed_rx = 0
        for filename in self.file_paths:
            print('Reading file RX PRR: ', filename)
            data = pd.read_csv(filename)
            tot_correct_rx = 0
            tot_failed_rx = 0
            for (err_pkts, nrx) in zip(data['N_ERR_PKTS'], data['N_RX']):
                err_pkts = int(err_pkts)
                nrx = int(nrx)
                tot_correct_rx += (nrx - err_pkts)
                tot_failed_rx += err_pkts
        
  
            if not self.is_bv:
                print('\nThis statistics is without bit voting\n')
            print(f'Total correct receptions: {tot_correct_rx}\n')
            print(f'Total failed receptions: {tot_failed_rx}\n')
            print(f'Packet reception rate:  {(tot_correct_rx/max(1, (tot_correct_rx+tot_failed_rx)))*100}\n')

            if self.tempwise:
                temp = filename.split('/')[-1].split('_')[-1].split('.')[0]
                if temp not in self.stat_dict:
                    self.stat_dict[temp] = {}
                    self.stat_dict[temp]['PRR'] = (tot_correct_rx/max(1, (tot_correct_rx+tot_failed_rx)))*100
                else:
                    self.stat_dict[temp]['PRR'] = (tot_correct_rx/max(1, (tot_correct_rx+tot_failed_rx)))*100
            else:
                self.stat_dict['PRR'] = (tot_correct_rx/max(1, (tot_correct_rx+tot_failed_rx)))*100


    
    def calc_rx_pdr(self) -> None:
        """
        This function calculates the packet delivery rate

        - PDR = rate of correct, unique packets received at L3.
        - Unique packets sent every 3rd round.
        - If a packet is received correctly in any of its rounds, that is considered
            a successful reception. It is only failed if a packet with a certain ID
            is never successfully received.

        :return: None
        """
        tot_correct_deliveries = 0
        tot_failed_deliveries = 0
        for filename in self.file_paths:
            print('Reading file RX PDR: ', filename)
            data = pd.read_csv(filename)
            tot_correct_deliveries = 0
            tot_failed_deliveries = 0
            for (err_pkts, nrx) in zip(data['N_ERR_PKTS'], data['N_RX']):
                err_pkts = int(err_pkts)
                nrx = int(nrx)

                if (nrx - err_pkts) > 0:
                    tot_correct_deliveries += 1
                else:
                    # print(nrx - err_pkts)
                    tot_failed_deliveries += 1
            
            tot_correct_deliveries = math.ceil(tot_correct_deliveries/3)
            tot_failed_deliveries = math.floor(tot_failed_deliveries/3)
            print(f'Total correct deliveries: {tot_correct_deliveries}\n')
            print(f'Total failed deliveries: {tot_failed_deliveries}\n')
            print(f'Packet delivery rate:  {(tot_correct_deliveries/max(1, (tot_correct_deliveries+tot_failed_deliveries)))*100}\n')

            if self.tempwise:
                temp = filename.split('/')[-1].split('_')[-1].split('.')[0]
                if temp not in self.stat_dict:
                    self.stat_dict[temp] = {}
                    self.stat_dict[temp]['PDR'] = (tot_correct_deliveries/max(1, (tot_correct_deliveries+tot_failed_deliveries)))*100
                else:
                    self.stat_dict[temp]['PDR'] = (tot_correct_deliveries/max(1, (tot_correct_deliveries+tot_failed_deliveries)))*100
            else:
                self.stat_dict['PDR'] = (tot_correct_deliveries/max(1, (tot_correct_deliveries+tot_failed_deliveries)))*100

    def calc_error_corrected_error_positions(self) -> None:
        """
        This function calculates the frequency of errors at each bit position and also notes 
        whether that errors were corrected or not from the CSV files.
        
        :return: None
        """
        # Read the data from the csv file
        for filename in self.file_paths:
            print('Reading file Corr_Err_positions: ', filename)
            data = pd.read_csv(filename)
            # Extract the error positions
            self.error_positions = {}
            self.corrected_error_positions = {}
            for (err_poses, ok, rnd) in zip(data["ERRS"], data["BV_SCS_FLAG"], data["ROUND"]):
                indices = err_poses.strip("\{\}").split(";")
                if len(indices) > 0:
                    for index in indices:
                        pair = index.split(":")
                        if len(pair) > 1:
                            i = int(pair[0])

                            # stop in case of errors
                            try:
                                freq = int(pair[1])
                            except:
                                print("err in rnd ", rnd, " index ", i)
                                exit()

                            if(i > 2040 or freq > 6):
                                print("err in rnd ", rnd, " index ", i)
                                exit()

                            if i not in self.error_positions:
                                self.error_positions[i] = freq
                            else:
                                self.error_positions[i] += freq

                            if ok == 1 and i not in self.corrected_error_positions:
                                self.corrected_error_positions[i] = freq
                            elif ok == 1 and i in self.corrected_error_positions:
                                self.corrected_error_positions[i] += freq
            if self.tempwise:
                temp = filename.split('/')[-1].split('_')[-1].split('.')[0]
                if temp not in self.error_positions_tempwise:
                    self.error_positions_tempwise[temp] = self.error_positions
                    self.corrected_error_positions_tempwise[temp] = self.corrected_error_positions
                else:
                    self.error_positions_tempwise[temp] = self.error_positions
                    self.corrected_error_positions_tempwise[temp] = self.corrected_error_positions
    
    def calc_error_positions(self) -> None:
        """
        This function calculates the error positions and frequency of error at that bit from the CSV files. 
        :return: None
        """
        # Read the data from the csv file
        for filename in self.file_paths:
            print('Reading file Calc Error Positions: ', filename)
            data = pd.read_csv(filename)
            # Extract the error positions
            self.error_positions = {}
            for (err_poses, rnd) in zip(data["ERRS"], data["ROUND"]):
                indices = err_poses.strip("\{\}").split(";")
                if len(indices) > 0:
                    for index in indices:
                        pair = index.split(":")
                        if len(pair) > 1:
                            i = int(pair[0])

                            # stop in case of errors
                            try:
                                freq = int(pair[1])
                            except:
                                print("err in rnd ", rnd, " index ", i)
                                exit()

                            if(i > 2040 or freq > 6):
                                print("err in rnd ", rnd, " index ", i)
                                exit()

                            if i not in self.error_positions:
                                self.error_positions[i] = freq
                            else:
                                self.error_positions[i] += freq
            
            if self.tempwise:
                temp = filename.split('/')[-1].split('_')[-1].split('.')[0]
                if temp not in self.error_positions_tempwise:
                    self.error_positions_tempwise[temp] = self.error_positions
                    # self.corrected_error_positions_tempwise[temp] = self.corrected_error_positions
                else:
                    self.error_positions_tempwise[temp] = self.error_positions
                    # self.corrected_error_positions_tempwise[temp] = self.corrected_error_positions
    
    def get_corrected_error_positions_dictionary(self) -> None:
        """
        This function returns the corrected error positions dictionary
        :return: corrected_error_positions
        """
        return self.corrected_error_positions

    def get_error_positions_dictionary(self) -> None:
        """
        This function returns the error positions dictionary
        :return: error_positions
        """
        return self.error_positions

    def get_corrected_error_positions_tempwise_dictionary(self) -> None:
        """
        This function returns the corrected error positions dictionary
        :return: corrected_error_positions
        """
        return self.corrected_error_positions_tempwise
    
    def get_error_positions_tempwise_dictionary(self) -> None:
        """
        This function returns the error positions dictionary
        :return: error_positions
        """
        return self.error_positions_tempwise