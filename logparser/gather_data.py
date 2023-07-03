import re
import datetime
import pandas as pd
import os
import sys

# usage: python3 gather_data_2.py <log_dir>

class FileReader:
    """
    This class reads the files in the directory and stores the file paths in a dictionary
    """
    def __init__(self) -> None:
        self.filedirectory = {}
        self.window_number = 0
    
    def get_directories(self, directory_path):
        """
        This function gets the directories in the Logoutput folder and stores them in a dictionary
        :return: None
        """
        # Specify the directory path
        sub_dirs = sorted(os.listdir(directory_path))
        # Iterate over the subdirectories
        for name in sub_dirs:
            if os.path.isdir(os.path.join(directory_path, name)):
                print('------------Adding Directory: ', os.path.join(directory_path, name), ' ------------')
                self.filedirectory[os.path.join(directory_path, name)] = []
    
    def get_directory_paths(self, directory_path):
        """
        This function reads the files in the directory and stores the file paths in a dictionary
        :return: None
        """
        self.get_directories(directory_path)
        for dir in self.filedirectory.keys():
            file_names = os.listdir(dir)
            # Sort the file names based on modification time
            sorted_file_names = sorted(file_names, key=lambda x: os.path.getmtime(os.path.join(dir+'/'+x)))
            # Read the files in the sorted order
            for filename in sorted_file_names:
                # Construct the full file path
                file_path = os.path.join(dir+'/'+filename)
                print('------------Adding file: ', file_path, ' ------------')
                # Check if the path is a file
                if os.path.isfile(file_path):
                    self.filedirectory[dir].append(file_path)
        return self.filedirectory
    
class Parser:
    """
    This class uses a File Reader to read the files
    and then parses the data and stores it in CSV files
    """
    def __init__(self) -> None:
        self.all_file_paths={}
        self.data_dict = {}
        self.data_frame = pd.DataFrame(columns=["TIMESTAMP", "ROUND", "N_RX", "N_ERR_PKTS", "BV_COUNT", "BV_SUCCESS_FLAG","ERRORS", "SLOTS"])
        self.dataframe_number = 0
        self.fr = FileReader()
        self.current_directory = ''

    def initalize_round(self):
        """
        This function initializes the data dictionary for a new round
        :return: None
        """
        self.data_dict = {'TIMESTAMP': '', 'ROUND': 0, "N_RX": 0, "N_ERR_PKTS": 0, "BV_COUNT": 0, "BV_SUCCESS_FLAG": -1,"ERRORS": {}, "SLOTS": ''}

    def read_complete_file(self, file_path):
        """
        This function reads the complete file and returns the logs
        :param file_path: path of the file
        :return: list of lines in the file
        """
        with open(file_path, 'r') as f:
            logs = f.readlines()
        return logs
    
    def get_all_path(self, directory):
        self.all_file_paths = self.fr.get_directory_paths(directory)
        return self.all_file_paths

    def write_to_csv_file(self):
        """
        This function writes the data frame to a csv file
        :return: None
        """
        if not os.path.exists(self.current_directory+'/CSVFiles'):
            os.makedirs(self.current_directory+'/CSVFiles')
        fl_nm = self.current_directory+'/CSVFiles/data'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+str(self.data_dict["ROUND"])+'.csv'
        print('---------Writing to file: ', fl_nm,' ----------')
        print(self.data_frame.head(5))
        print(self.data_frame.tail(5))
        self.data_frame.to_csv(fl_nm, index=False)

    def reset_dataframe(self):
        """
        This function resets the data frame
        :return: None
        """
        self.data_frame = pd.DataFrame(columns=["TIMESTAMP", "ROUND", "N_RX", "N_ERR_PKTS","BV_COUNT", "BV_SUCCESS_FLAG","ERRORS", "SLOTS"])
    
    def update_dataframe(self):
        """ 
        This function updates the data frame with the new round data dictionary
        :return: None
        """
        new_df = pd.DataFrame([self.data_dict])
        self.data_frame = pd.concat([self.data_frame, new_df], ignore_index=True)
        self.dataframe_number += 1

    def parsing_logic_new(self,log_strings):
        ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
        for log in log_strings:
            self.initalize_round()
            log_data = ansi_escape.sub('', log)
            if "Packet" not in log_data:
                log_data = log.split(" ")

                time_stamp = log_data[0]+" "+log_data[1]
                self.data_dict["TIMESTAMP"] = time_stamp
            
                log_data = log_data[2].split(",")
                
                if len(log_data) == 8:
                    round_num = log_data[1].split(":")[-1]
                    print("---------------------round_num: ", round_num, "----------------------")
                    self.data_dict["ROUND"] = round_num
                    print("TIMESTAMP: ", self.data_dict["TIMESTAMP"])
                    
                    n_rx = log_data[2].split(":")[-1]
                    print("no. of receives: ", n_rx)
                    self.data_dict["N_RX"] = n_rx
                    
                    n_err_pkts = log_data[3].split(":")[-1]
                    print("err_pkts: ", n_err_pkts)
                    self.data_dict["N_ERR_PKTS"] = n_err_pkts
                    
                    bv_count = log_data[4].split(":")[-1]
                    print("counting bv: ", bv_count)
                    self.data_dict["BV_COUNT"] = bv_count
                    
                    bv_success_flag = log_data[5].split(":")[-1]
                    print("BV success flag: ", bv_success_flag)
                    self.data_dict["BV_SUCCESS_FLAG"] = bv_success_flag
                    
                    errors = log_data[6].split("ERRS:")[-1]
                    print('errors: ', errors)
                    self.data_dict["ERRORS"] = errors
                    
                    slots = log_data[7].split(":")[-1].strip()
                    print("slots: ", slots)
                    self.data_dict["SLOTS"] = slots
                    print(self.data_dict)
                    self.update_dataframe()
                    print('-----------------------------------------------------------------------')

        self.write_to_csv_file()
        self.reset_dataframe()

def main(argv):
    log_parser = Parser()

    if len(argv) < 1:
        print("usage: python3 gather_data_2.py <log_dir>")
        exit(1)
    elif not os.path.exists(argv[0]):
        print("please enter a valid directory")
        print("usage: python3 gather_data_2.py <log_dir>")
        exit(1)

    tot_paths = log_parser.get_all_path(argv[0])
    for subdir, paths in tot_paths.items():
        log_parser.current_directory = subdir
        for path in paths:
            log_strings = log_parser.read_complete_file(path)
            log_parser.parsing_logic_new(log_strings)

if __name__ =='__main__':
    main(sys.argv[1:])