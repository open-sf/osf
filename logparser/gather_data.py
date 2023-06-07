import re
import datetime
import pandas as pd
import os

class Parser:
    """
    This class parses the log file and extracts the required information
    """
    def __init__(self):
        self.data_dict = {}
        self.error_pkts = []
        self.correct_pkts = []
        self.correct_pkt_received = False
        self.num_rounds = 0
        self.data_frame = pd.DataFrame(columns=['Round Number','Total Errors', 'ERR', 'OK!', 'FAIL!', 'BV_COUNT', 'ERR_POSITIONS'])
        self.dataframe_number = 0
        self.subdirectory = ''
    
    def get_directory_path(self):
        # Specify the directory path
        directory_path = "./Logoutput/"

        # Iterate over the subdirectories
        for root, dirs, files in os.walk(directory_path):
            for directory in dirs:
                # Print the absolute path of each subdirectory
                subdirectory_path = os.path.join(root, directory)
                self.subdirectory = subdirectory_path
                yield subdirectory_path

    def read_files(self):
        directory_path = self.get_directory_path()
        for path in directory_path:
            file_names = os.listdir(path)

            # Sort the file names based on modification time
            sorted_file_names = sorted(file_names, key=lambda x: os.path.getmtime(os.path.join(path, x)))

            # Read the files in the sorted order
            for filename in sorted_file_names:
                # Construct the full file path
                file_path = os.path.join(path, filename)
                print('------------Opening file: ', file_path, ' ------------')
                # Check if the path is a file
                if os.path.isfile(file_path):
                        yield file_path

    def initalize_round(self):
        """
        This function initializes the data dictionary for a new round
        :return: None
        """
        self.num_rounds += 1
        self.data_dict = {'Round Number': self.num_rounds, 'Total Errors': 0, 'ERR': [], 'OK!': 0, 'FAIL!': 0, 'BV_COUNT': [], 'ERR_POSITIONS': []}
    
    def reset_dataframe(self):
        """
        This function resets the data frame
        :return: None
        """
        
        self.data_frame = pd.DataFrame(columns=['Round Number','Total Errors', 'ERR', 'OK!', 'FAIL!', 'BV_COUNT', 'ERR_POSITIONS'])
    
    def update_dataframe(self):
        """ 
        This function updates the data frame with the new round data dictionary
        :return: None
        """
        new_df = pd.DataFrame([self.data_dict])
        self.data_frame = pd.concat([self.data_frame, new_df], ignore_index=True)
        self.dataframe_number += 1
    
    def read_complete_file(self, file_path):
        """
        This function reads the complete file and returns the logs
        :param file_path: path of the file
        :return: list of lines in the file
        """
        with open(file_path, 'r') as f:
            logs = f.readlines()
        return logs

    def write_to_csv_file(self):
        """
        This function writes the data frame to a csv file
        :return: None

        """
        if not os.path.exists(self.subdirectory+'/CSVFiles'):
            os.makedirs(self.subdirectory+'/CSVFiles')
        fl_nm = self.subdirectory+'/CSVFiles/data'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+str(self.num_rounds)+'.csv'
        print('---------Writing to file: ', fl_nm,' ----------')
        print(self.data_frame.head(5))
        print(self.data_frame.tail(5))
        self.data_frame.to_csv(fl_nm, index=False)

    def find_error_positions(self):
        """
        
        This function finds the positions of erroneous bit in the error packets
        :return: dictionary of error positions and their counts
        
        """
        err_bit_pos = {}
        print('Correct Packet: \n', self.correct_pkts)
        print("Packets with errors:\n", self.error_pkts)

        try:

            if len(self.error_pkts) > 0 and len(self.correct_pkts) > 0:
                correct_packet_bytes = bytearray.fromhex(self.correct_pkts[0].replace(" ", ""))

                # Calculate the number of bit errors and track the positions
                for err_pkt in self.error_pkts:
                    bit_errors = 0
                    error_packet_bytes = bytearray.fromhex(err_pkt.replace(" ", ""))
                    for i, (error_byte, correct_byte) in enumerate(zip(error_packet_bytes, correct_packet_bytes)):
                        diff_bits = error_byte ^ correct_byte  # XOR to find differing bits
                        if diff_bits != 0:
                            bit_errors += bin(diff_bits).count("1")  # Count the number of set bits
                            # Track the positions of the differing bits
                            for bit_pos in range(8):
                                if diff_bits & (1 << bit_pos):
                                    error_pos = i * 8 + (bit_pos - 7)
                                    if error_pos not in err_bit_pos:
                                        err_bit_pos[error_pos] = 1
                                    else:
                                        err_bit_pos[error_pos] += 1
        except ValueError:
            pass
        return err_bit_pos

    def get_error_positions(self):
        """
        This function gets the error positions in the error packets
        :return None
        """
        
        positions = self.find_error_positions()
        print("Error Positions", positions)
        positions = list(positions.keys())

        if 'ERR_POSITIONS' not in self.data_dict:
            self.data_dict['ERR_POSITIONS'] = positions
        else:
            if len(positions) > 0:
                for index in positions:
                    self.data_dict['ERR_POSITIONS'].append(index)

        #For the case when there is no correct reception at the end of the round
        #At that time, the error packets should be preserved till a correct reception is received or error correction is performed
        if len(self.correct_pkts) > 0:
            self.error_pkts = []


    def parsing_logic(self, log_strings):
        """
        This function parses the log strings, updates the data dictionary
        with reqruired information such as number of errors, number of correct packets, etc.
        and then updates the data frame and writes it to a csv file
        :param log_strings: list of log strings
        :return: None
        """
        for log_string in log_strings:
            log_string = log_string.strip()

            # Removing the ANSI escape characters
            ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
            log_string = ansi_escape.sub('', log_string)
            
            # Regular expression pattern to extract information from the log string
            pattern = r'\[(.*?)\] \[(.*?)\] (.*)'
            
            # Extracting information using regex pattern
            match = re.match(pattern, log_string)
            if match:
                timestamp = match.group(1) #Extracting timestamp
                packet_type = match.group(2).strip().split(':')[0].strip()  # Extracting the log type
                data = match.group(3).strip()  # Extracting the log message

                # Storing the information in a dictionary
                if packet_type == 'ERR':
                    print(packet_type, data)
                    if packet_type not in self.data_dict:
                        self.data_dict[packet_type] = [timestamp]
                        self.data_dict['Total Errors'] = len(self.data_dict['ERR'])
                        self.error_pkts.append(data)
                    else:
                        self.data_dict[packet_type].append(timestamp)
                        self.data_dict['Total Errors'] = len(self.data_dict['ERR'])
                        self.error_pkts.append(data)
                    print("Error packets till now: ", self.error_pkts)
                
                elif packet_type == 'OK!':
                    print(packet_type, data)
                    if packet_type not in self.data_dict:
                        self.data_dict[packet_type] = 1
                    else:
                        self.data_dict[packet_type] += 1

                elif packet_type == 'FAIL!':
                    print(packet_type, data)
                    if packet_type not in self.data_dict:
                        self.data_dict[packet_type] = 1
                    else:
                        self.data_dict[packet_type] += 1
                        
                elif packet_type == 'BV_COUNT':
                    print(packet_type, data)
                    if packet_type not in self.data_dict:
                        self.data_dict[packet_type] = [int(data.strip())]
                    else:
                        self.data_dict[packet_type].append(int(data.strip()))

                elif packet_type == 'PKT':
                    print(packet_type, data)
                    if not self.correct_pkt_received:
                        self.correct_pkts.append(data)
                        self.correct_pkt_received = True
                    print('Correct packets till now: ', self.correct_pkts)

                elif packet_type == 'nid':
                    print(packet_type, data)
                    self.get_error_positions()
                    self.correct_pkts = []
                    self.error_pkts = []
                    self.correct_pkt_received = False

                else:
                    if data == 'ROUND START':
                        #This part limits the size of the data frame to 1000 rows and writes it to a csv file
                        #Reset the data frame and start appending data to it again
                        print('Length of dataframe till now: ', len(self.data_frame))
                        if len(self.data_frame) == 200:
                            self.write_to_csv_file()
                            self.reset_dataframe()
                        self.initalize_round()
                        print('--------------- ', data, ' : ',self.num_rounds,' ---------------')
                    
                    elif data == 'ROUND END':
                        self.get_error_positions()
                        
                        print('Writing to dataframe number: ', self.dataframe_number, " for round: ", self.num_rounds)
                        print("Dataframe:\n",self.data_dict)

                        self.update_dataframe()
                        print('--------------- ',data,' : ', self.num_rounds, ' ---------------')
                print('--------------------------------------------------')    
        # self.write_to_csv_file()

if __name__ =='__main__':
    log_parser = Parser()
    file_path = log_parser.read_files()
    for path in file_path:
        log_strings = log_parser.read_complete_file(path)
        log_parser.parsing_logic(log_strings)
