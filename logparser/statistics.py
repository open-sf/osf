import os
import pandas as pd
from read_log_files import FileReader

class StatsCalculator:

    def __init__(self, dp) -> None:
        self.directory_path = dp
        self.fr = FileReader(self.directory_path)
        self.file_paths = self.fr.get_csv_file_paths()

    def add_line(self, str):
        """
        This function writes the observations to a file
        :return: None
        """
        physical_layer = self.directory_path.split('/')[-1]
        file_path = self.directory_path + f'/statistics{physical_layer}.txt'
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
        for filenames in self.file_paths.values():
            # total_bv_count = 0
            # num_oks = 0
            # avg_bv_count = 0
            for filename in filenames:
                data = pd.read_csv(filename)
                for (bv_count, ok) in zip(data['BV_COUNT'], data['BV_SUCCESS_FLAG']):
                    bv_count = int(bv_count)

                    if ok == 1:
                        total_bv_count += bv_count
                        num_oks += 1
        if num_oks > 0:
            avg_bv_count = total_bv_count/num_oks
            self.add_line(f'Avg. Rx before correction: {avg_bv_count}\n')
        else:
            self.add_line('No successful corrections')
    
    def calc_err_pkts_correct_pkt(self):
        """
        This function calculates the number of error packets and correct packets
        :return: None
        """
        tot_err_pkts = 0 
        tot_correct_pkts = 0
        for filenames in self.file_paths.values():
            for filename in filenames:
                data = pd.read_csv(filename)
                for (err, ok) in zip(data['N_ERR_PKTS'], data['BV_SUCCESS_FLAG']):
                    err = int(err)
                    ok = int(ok)
                    # print(err, bv_c)
                    # print('---------------------')
                    tot_err_pkts += err

                    if ok == 1:
                        tot_correct_pkts += err
        
        self.add_line(f'Total error packets: {tot_err_pkts}\n')
        self.add_line(f'Total packets corrected: {tot_correct_pkts}\n')
    
    def calc_rx_prr(self):
        """
        This function calculates the packet reception rate
        :return: None
        """
        
        #Total number of Correct Receptions per round is N_RX_PKTS - N_ERR_PKTS

        #Total number of failed receptions per round is N_ERR_PKTS + Missed packets
        #Which is calculated from slot string where in it is represented as C in the slot string

        tot_correct_rx = 0
        tot_failed_rx = 0
        for filenames in self.file_paths.values():
            for filename in filenames:
                data = pd.read_csv(filename)
                for (err_pkts, nrx, slt_str) in zip(data['N_ERR_PKTS'], data['N_RX'], data['SLOTS']):
                    err_pkts = int(err_pkts)
                    nrx = int(nrx)
                    slt_str = str(slt_str)
                    tot_correct_rx += (nrx - err_pkts)
                    tot_failed_rx += (err_pkts + slt_str.count('M'))
        
        self.add_line(f'Total correct receptions: {tot_correct_rx}\n')
        self.add_line(f'Total failed receptions: {tot_failed_rx}\n')
        self.add_line(f'Packet reception rate:  {(tot_correct_rx/(tot_correct_rx+tot_failed_rx))*100}\n')