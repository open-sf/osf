import math
import os
import pandas as pd
from read_log_files import FileReader

class StatsCalculator:

    def __init__(self, main_dp, log_dp) -> None:
        self.directory_path = main_dp
        self.fr = FileReader(log_dp)
        self.file_paths = self.fr.get_csv_file_paths()
        self.corrected_packets = 0

    def add_line(self, str):
        """
        This function writes the observations to a file
        :return: None
        """
        physical_layer = self.directory_path.split('/')[-1]
        file_path = self.directory_path + f'/statistics_{physical_layer}.txt'
        if not os.path.exists(file_path):
            with open(file_path, 'w') as f:
                f.write(str)
        else:
            with open(file_path, 'a') as f:
                f.write(str)
    
    def calc_avg_rx_before_correction(self):
        """
        This function calculates the average number of receptions taken to
        successfully correct an error.
        :return: None
        """
        total_bv_count = 0
        num_oks = 0
        avg_bv_count = 0
        # for filenames in self.file_paths.values():
            # total_bv_count = 0
            # num_oks = 0
            # avg_bv_count = 0
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            for (bv_count, ok) in zip(data['BV_COUNT'], data['BV_SUCCESS_FLAG']):
                bv_count = int(bv_count)

                if ok == 1:
                    total_bv_count += bv_count
                    num_oks += 1
        self.add_line('\nThis statistics is with Bit voting enaled\n')
        if num_oks > 0:
            avg_bv_count = total_bv_count/num_oks
            self.add_line(f'Avg. Rx before correction: {avg_bv_count}\n')
        else:
            self.add_line('No successful corrections\n')
    
    def calc_total_nrxs(self):
        """
        This function calculates the total number of receptions
        :return: None
        """
        tot_nrx = 0
        # for filenames in self.file_paths.values():
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            for nrx in data['N_RX']:
                nrx = int(nrx)
                tot_nrx += nrx
        self.add_line(f'Total number of receptions: {tot_nrx}\n')
    
    # We are ignoring this function for now
    def calc_err_pkts_correct_pkt(self):
        """
        This function calculates the number of error packets and correct packets
        :return: None
        """
        tot_err_pkts = 0 
        tot_correct_pkts = 0
        tot_correct_failed_pkts = 0
        # for filenames in self.file_paths.values():
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            for (err, ok) in zip(data['N_ERR_PKTS'], data['BV_SUCCESS_FLAG']):
                err = int(err)
                ok = int(ok)
                # print(err, bv_c)
                # print('---------------------')
                tot_err_pkts += err

                if ok == 1:
                    tot_correct_pkts += err
                elif ok == 0:
                    tot_correct_failed_pkts += err
        self.corrected_packets = tot_correct_pkts
        self.add_line(f'Total error packets: {tot_err_pkts}\n')
        self.add_line(f'Total packets corrected: {tot_correct_pkts}\n')
        self.add_line(f'Total packets correction failed: {tot_correct_failed_pkts}\n')
        self.add_line(f'Percentage of packets corrected: {(tot_correct_pkts/max(1,(tot_err_pkts + tot_correct_pkts)))*100}\n')
    
    # BV count no longer in logs
    def calc_suc_fail_bv_count(self):
        """
        This function calculates the number of successful and failed bit voting attempts
        :return: None
        """
        
        tot_suc_bv = 0
        tot_fail_bv = 0
        tot_bv = 0 # currently unused?
        # for filenames in self.file_paths.values():
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            for (bv_count, ok) in zip(data['BV_COUNT'], data['BV_SUCCESS_FLAG']):
                bv_count = int(bv_count)
                ok = int(ok)
                if ok == 1:
                    tot_suc_bv += 1
                elif ok == 0:
                    tot_fail_bv += 1
                if bv_count == 255:
                    tot_bv += bv_count  
            
        self.add_line(f'Total successful bit voting attempts: {tot_suc_bv}\n')
        self.add_line(f'Total failed bit voting attempts: {tot_fail_bv}\n')
        self.add_line(f'Percentage of successful bit voting attempts: {(tot_suc_bv/max(1,(tot_suc_bv+tot_fail_bv)))*100}\n')
    
    def calc_rx_prr(self):
        """
        This function calculates the packet reception rate
        :return: None
        """
        
        #Total number of Correct Receptions per round is N_RX_PKTS - N_ERR_PKTS

        #Total number of failed receptions per round is N_ERR_PKTS + Missed packets
        #Which is calculated from slot string where in it is represented as M in the slot string

        tot_correct_rx = 0
        tot_missed_rx = 0
        tot_failed_rx = 0
        # for filenames in self.file_paths.values():
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            for (err_pkts, nrx, slt_str) in zip(data['N_ERR_PKTS'], data['N_RX'], data['SLOTS']):
                err_pkts = int(err_pkts)
                nrx = int(nrx)
                slt_str = str(slt_str)
                tot_correct_rx += (nrx - err_pkts)
                # tot_correct_rx += (slt_str.count('R'))
                tot_failed_rx += err_pkts
                tot_missed_rx += (slt_str.count('M'))
            # tot_failed_rx += tot_missed_rx
        # tot_correct_rx += self.corrected_packets
        # tot_failed_rx -= self.corrected_packets
        self.add_line(f'Total correct receptions: {tot_correct_rx}\n')
        self.add_line(f'Total failed receptions: {tot_failed_rx}\n')
        self.add_line(f'Total missed receptions: {tot_missed_rx}\n')
        # self.add_line(f'Packet reception rate:  {(tot_correct_rx/max(1, (tot_correct_rx+tot_failed_rx)))*100}\n')
        # Added missed slots to calculation
        self.add_line(f'Packet reception rate:  {(tot_correct_rx/max(1, (tot_correct_rx+tot_failed_rx+tot_missed_rx)))*100}\n')
    
    # Logs different based on BV enabled
    # Looks like we are probably able to use one function for PRR for both BV
    # enabled and BV disabled
    def calc_rx_prr_no_bv(self):
        """
        This function calculates the packet reception rate without bit voting
        :return: None
        """
        
        #Total number of Correct Receptions per round is N_RX_PKTS - N_ERR_PKTS

        #Total number of failed receptions per round is N_ERR_PKTS + Missed packets
        #Which is calculated from slot string where in it is represented as C in the slot string

        tot_correct_rx = 0
        tot_failed_rx = 0
        tot_missed_rx = 0
        # for filenames in self.file_paths.values():
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            for slots in data["SLOTS"]:
                slots = str(slots)
                print("slots", slots)
                # print("R count: ",slots.count('R'))
                # print("M count: ",slots.count('M'))
                # print("C count: ",slots.count('C'))
                tot_correct_rx += slots.count('R')
                tot_failed_rx +=  slots.count('C')
                tot_missed_rx += slots.count('M')
            # tot_failed_rx += tot_missed_pkts

        self.add_line('\nThis statistics is without bit voting\n')
        self.add_line(f'Total correct receptions: {tot_correct_rx}\n')
        self.add_line(f'Total failed receptions: {tot_failed_rx}\n')
        self.add_line(f'Total missed packets: {tot_missed_rx}\n')
        # self.add_line(f'Packet reception rate:  {(tot_correct_rx/max(1, (tot_correct_rx+tot_failed_rx)))*100}\n')
        # Added missed slots to calculation
        self.add_line(f'Packet reception rate:  {(tot_correct_rx/max(1, (tot_correct_rx+tot_failed_rx+tot_missed_rx)))*100}\n')

    """
    - PDR = rate of correct, unique packets received at L3.
    - Unique packets sent every 3rd round.
    - If a packet is received correctly in any of its rounds, that is considered
      a successful reception. It is only failed if a packet with a certain ID
      is never successfully received.
    """
    def calc_rx_pdr(self):
        """
        This function calculates the packet delivery rate
        :return: None
        """
        tot_correct_deliveries = 0
        tot_failed_deliveries = 0
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            # for (err_pkts, nrx, sl_str) in zip(data['N_ERR_PKTS'], data['N_RX'], data['SLOTS']):
            for (err_pkts, nrx) in zip(data['N_ERR_PKTS'], data['N_RX']):
                err_pkts = int(err_pkts)
                nrx = int(nrx)
                # sl_str = str(sl_str)

                if (nrx - err_pkts) > 0:
                    tot_correct_deliveries += 1
                else:
                    tot_failed_deliveries += 1

                # tot_correct_deliveries += (nrx - err_pkts)
                # tot_failed_deliveries += err_pkts
                # tot_failed_deliveries += sl_str.count('M')
            
        tot_correct_deliveries = math.ceil(tot_correct_deliveries/3)
        tot_failed_deliveries = math.floor(tot_failed_deliveries/3)
        self.add_line(f'Total correct deliveries: {tot_correct_deliveries}\n')
        self.add_line(f'Total failed deliveries: {tot_failed_deliveries}\n')
        self.add_line(f'Packet delivery rate:  {(tot_correct_deliveries/max(1, (tot_correct_deliveries+tot_failed_deliveries)))*100}\n')
    
    # Logs different based on BV enabled
    # Looks like we could probably use this method for both BV enabled and
    # BV disabled
    def calc_rx_pdr_no_bv(self):
        """
        This function calculates the packet delivery rate without bit voting
        :return: None
        """
        tot_correct_deliveries = 0
        tot_failed_deliveries = 0
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            for slots in data["SLOTS"]:
                slots = str(slots)

                if 'R' in slots:
                    tot_correct_deliveries += 1
                else:
                    tot_failed_deliveries += 1

                # tot_correct_deliveries += slots.count('R')
                # tot_failed_deliveries +=  slots.count('C')
                # tot_failed_deliveries += slots.count('M')
            
        tot_correct_deliveries = math.ceil(tot_correct_deliveries/3)
        tot_failed_deliveries = math.floor(tot_failed_deliveries/3)
        self.add_line('\nThis statistics is without bit voting\n')
        self.add_line(f'Total correct deliveries: {tot_correct_deliveries}\n')
        self.add_line(f'Total failed deliveries: {tot_failed_deliveries}\n')
        self.add_line(f'Packet delivery rate:  {(tot_correct_deliveries/max(1, (tot_correct_deliveries+tot_failed_deliveries)))*100}\n')