import re
import datetime
import pandas as pd
import time
import os
import io
import subprocess
def write_to_csv_file(data_frame, idx):
    """
    This function writes the data frame to a csv file
    :param data_frame: data frame to be written to a csv file
    :param idx: index of the csv file
    :return: None

    """
    data_frame.to_csv('./CSVFiles/data'+str(idx)+'.csv', index=False)

def convert_to_binary(hex_string):
    """
    This function converts the hexadecimal packet to binary
    :param sequence: hexadecimal packet sequence to be converted to binary
    :return: binary sequence(string) of the sequence
    """

    hex_to_bin_table = {
        '0': '0000', '1': '0001', '2': '0010', '3': '0011',
        '4': '0100', '5': '0101', '6': '0110', '7': '0111',
        '8': '1000', '9': '1001', 'A': '1010', 'B': '1011',
        'C': '1100', 'D': '1101', 'E': '1110', 'F': '1111'
    }

    hex_string = hex_string.replace(" ", "")  # Remove spaces from the input string
    binary_string = ''

    for char in hex_string:
        char = char.upper()
        if char in hex_to_bin_table:
            binary_string += hex_to_bin_table[char]
        else:
            raise ValueError(f"Invalid hexadecimal digit: {char}")

    return binary_string

def get_binary_sequences(error_pkts, correct_pkts):
    """
    This function converts the hexadecimal packets to binary
    :param error_pkts: list of hexadecimal error packets
    :param correct_pkts: list of hexadecimal correct packets
    :return: list of binary error packets and list of binary correct packets
    
    """
    
    # print('Inside get_binary_sequences')
    err_seq = []
    correct_seq = []        
    for pkt in error_pkts:
        err_seq.append(convert_to_binary(pkt))
    
    for pkt in correct_pkts:    
        correct_seq.append(convert_to_binary(pkt))
    return err_seq, correct_seq

def find_error_positions(error_pkts, correct_pkts):
    """
    
    This function finds the error positions in the error packets
    :param error_pkts: list of hexadecimal error packets
    :param correct_pkts: list of hexadecimal correct packets
    :return: dictionary of error positions and their counts
    
    """
    
    err_seq = []
    correct_seq = []
    err_bit_pos = {}
    print('Correct Packet: \n', correct_pkts)
    print("Packets with errors:\n", error_pkts)

    if len(error_pkts) > 0 and len(correct_pkts) > 0:
        err_seq, correct_seq = get_binary_sequences(error_pkts, correct_pkts)
        if len(err_seq[0]) == len(correct_seq[0]):
            for seq in err_seq:
                for i in range(len(seq)):
                    if seq[i] != correct_seq[0][i]:
                        if i not in err_bit_pos:
                            err_bit_pos[i] = 1
                        else:
                            err_bit_pos[i] += 1
    return err_bit_pos

def get_error_positions(error_pkts, correct_pkts, data_dict, round_num):
    """
    This function finds the error positions in the error packets
    :param error_pkts: list of hexadecimal error packets
    :param correct_pkts: list of hexadecimal correct packets
    :param data_dict: dictionary to store the error positions
    :param round_num: round number
    :return: dictionary of error positions and their counts
    :return: list of error packets
    :return: list of correct packets
    """
    
    positions = find_error_positions(error_pkts, correct_pkts)
    print("Printing poses", positions)
    positions = list(positions.keys())
    data_dict[round_num]['ERR_POSITIONS'] = positions
    error_pkts = []
    correct_pkts = []
    return data_dict, error_pkts, correct_pkts


# def follow(thefile):
#     '''generator function that yields new lines in a file
#     '''
#     # seek the end of the file
#     thefile.seek(0, os.SEEK_END)

#     # start infinite loop
#     while True:
#         # read last line of file
#         line = thefile.readline()
#         # sleep if file hasn't been updated
#         if not line:
#             time.sleep(0.1)
#             continue

#         yield line

def follow(filepath):

    # Create a subprocess to continuously read updates from the log file
    powershell_command = f'Get-Content -Wait -Tail 1 "{filepath}"'
    tail_process = subprocess.Popen(['powershell.exe', '-Command', powershell_command], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)

    # Continuously read and parse the log file updates
    while True:
        # Read the new data from the pipe
        new_data = tail_process.stdout.readline().decode().strip()

        # If no new data is available, wait for a short period and continue
        if not new_data:
            time.sleep(0.2)
            continue
        yield new_data

def parse_file(file_path):
    """
    This function parses the log file and extracts the required information
    :param file_path: path of the log file
    :return: None

    """
    
    idx = 0
    # with open(file_path, 'r') as f:
    # log_file = io.open(file_path, buffering=1, newline=None)
    dataframe_number = 0
    log_strings = follow(file_path)
    num_rounds = 0
    data_dict = {}
    error_pkts = []
    correct_pkts = []
    correct_pkt_received = False
    data_frame = pd.DataFrame(columns=['Total Errors', 'ERR', 'OK!', 'FAIL!', 'BV_COUNT', 'ERR_POSITIONS'])
    for log_string in log_strings:
        log_string = log_string.strip()
        # Removing the ANSI escape characters
        ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
        log_string = ansi_escape.sub('', log_string)
        # Regular expression pattern to extract information from the log string
        pattern = r'\[(.*?)\] (.*)'
        # Extracting information using regex pattern
        match = re.match(pattern, log_string)
        if match:
            packet_type = match.group(1).strip().split(':')[0].strip()  # Extracting the packet type
            data = match.group(2).strip()  # Extracting the log message

            # Storing the information in a dictionary
            if packet_type == 'ERR':
                print(packet_type, data)
                if packet_type not in data_dict[num_rounds]:
                    data_dict[num_rounds][packet_type] = [datetime.datetime.now()]
                    data_dict[num_rounds]['Total Errors'] = len(data_dict[num_rounds]['ERR'])
                    error_pkts.append(data)
                else:
                    data_dict[num_rounds][packet_type].append(datetime.datetime.now())
                    data_dict[num_rounds]['Total Errors'] = len(data_dict[num_rounds]['ERR'])
                    error_pkts.append(data)

            elif packet_type == 'OK!':
                print(packet_type, data)
                if packet_type not in data_dict[num_rounds]:
                    data_dict[num_rounds][packet_type] = 1
                else:
                    data_dict[num_rounds][packet_type] += 1

            elif packet_type == 'FAIL!':
                print(packet_type, data)
                if packet_type not in data_dict[num_rounds]:
                    data_dict[num_rounds][packet_type] = 1
                else:
                    data_dict[num_rounds][packet_type] += 1
                    
            elif packet_type == 'BV_COUNT':
                print(packet_type, data)
                if packet_type not in data_dict[num_rounds]:
                    data_dict[num_rounds][packet_type] = [int(data.strip())]
                else:
                    data_dict[num_rounds][packet_type].append(int(data.strip()))

            elif packet_type == 'PKT':
                print(packet_type, data)
                if not correct_pkt_received:
                    correct_pkts.append(data)
                    correct_pkt_received = True

            elif packet_type == 'nid':
                print(packet_type, data)
                data_dict, error_pkts, correct_pkts = get_error_positions(error_pkts, correct_pkts, data_dict, num_rounds)
                correct_pkt_received = False
            else:
                if data == 'ROUND START':
                    #This part limits the size of the data frame to 1000 rows and writes it to a csv file
                    #Reset the data frame and start appending data to it again
                    print('Length of dataframe: ', len(data_frame))
                    if len(data_frame) == 200:
                        write_to_csv_file(data_frame, idx)
                        data_frame = pd.DataFrame(columns=['Total Errors', 'ERR', 'OK!', 'FAIL!', 'BV_COUNT', 'ERR_POSITIONS'])
                        idx += 1
                    num_rounds += 1
                    data_dict[num_rounds] = {}
                elif data == 'ROUND END':
                    if num_rounds not in data_dict:
                        data_dict[num_rounds] = {}           
                    print('Writing to dataframe number: ', dataframe_number, " for round: ", num_rounds)
                    print("Dataframe:\n",data_dict[num_rounds])
                    new_df = pd.DataFrame([data_dict[num_rounds]])
                    data_frame = pd.concat([data_frame, new_df], ignore_index=True)
                    dataframe_number += 1
            print('--------------------------')


if __name__ == '__main__':
    # parse_file('C:/Users/AVuser/Desktop/logs/putty_dst.log')
    parse_file('D:/Thesis Research/Logs/putty_dst.log')