import re
import datetime
import pandas as pd
import time
import os
import io
import subprocess

def read_complete_file(file_path):
    """
    This function reads the complete file and returns the data
    This function is for debug purpose only
    :param file_path: path of the file
    :return: list of lines in the file
    """
    with open(file_path, 'r') as f:
        data = f.readlines()
    return data

def write_to_csv_file(data_frame, idx):
    """
    This function writes the data frame to a csv file
    :param data_frame: data frame to be written to a csv file
    :param idx: index of the csv file
    :return: None

    """

    data_frame.to_csv('./Temporary_CSV/data'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.csv', index=False)
    #  (For Debugging)
    # data_frame.to_csv('./data'+str(idx)+'.csv', index=False)

def find_error_positions(error_pkts, correct_pkts):
    """
    
    This function finds the positions of erroneous bit in the error packets
    :param error_pkts: list of hexadecimal error packets
    :param correct_pkts: list of hexadecimal correct packets
    :return: dictionary of error positions and their counts
    
    """
    err_bit_pos = {}
    print('Correct Packet: \n', correct_pkts)
    print("Packets with errors:\n", error_pkts)

    try:

        if len(error_pkts) > 0 and len(correct_pkts) > 0:
            correct_packet_bytes = bytearray.fromhex(correct_pkts[0].replace(" ", ""))

            # Calculate the number of bit errors and track the positions
            for err_pkt in error_pkts:
                bit_errors = 0
                error_packet_bytes = bytearray.fromhex(err_pkt.replace(" ", ""))
                for i, (error_byte, correct_byte) in enumerate(zip(error_packet_bytes, correct_packet_bytes)):
                    diff_bits = error_byte ^ correct_byte  # XOR to find differing bits
                    if diff_bits != 0:
                        bit_errors += bin(diff_bits).count("1")  # Count the number of set bits
                        # Track the positions of the differing bits
                        for bit_pos in range(8):
                            if diff_bits & (1 << bit_pos):
                                error_pos = i * 8 + bit_pos
                                if error_pos not in err_bit_pos:
                                    err_bit_pos[error_pos] = 1
                                else:
                                    err_bit_pos[error_pos] += 1
    except ValueError:
        pass
    return err_bit_pos

def get_error_positions(error_pkts, correct_pkts, data_dict, round_num):
    """
    This function gets the error positions in the error packets
    :param error_pkts: list of hexadecimal error packets
    :param correct_pkts: list of hexadecimal correct packets
    :param data_dict: dictionary to store the error positions
    :param round_num: round number
    :return: dictionary of error positions and their counts
    :return: list of error packets
    :return: list of correct packets
    """
    
    positions = find_error_positions(error_pkts, correct_pkts)
    print("Error Positions", positions)
    positions = list(positions.keys())

    if 'ERR_POSITIONS' not in data_dict[round_num]:
        data_dict[round_num]['ERR_POSITIONS'] = positions
    else:
        if len(positions) > 0:
            data_dict[round_num]['ERR_POSITIONS'].append(positions)

    #For the case when there is no correct reception at the end of the round
    #At that time, the error packets should be preserved till a correct reception is received or error correction is performed
    if len(correct_pkts) > 0:
        error_pkts = []
    return data_dict, error_pkts, correct_pkts


def parsing_logic(log_strings, num_rounds, data_dict, error_pkts, correct_pkts, correct_pkt_received, data_frame, dataframe_number):

    # # with open(file_path, 'r') as f:
    # # log_file = io.open(file_path, buffering=1, newline=None)
    # dataframe_number = 0
    # log_strings = follow(file_path)
    # # log_strings = read_complete_file(file_path)
    print('length of log strings: ', len(log_strings))
    for log_string in log_strings:
        # print(log_string)
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
            packet_type = match.group(2).strip().split(':')[0].strip()  # Extracting the packet type
            data = match.group(3).strip()  # Extracting the log message
            # print(timestamp)
            # print(packet_type)
            # print(data)
            # print('----------------------------------')
            # Storing the information in a dictionary
            if packet_type == 'ERR':
                print(packet_type, data)
                if packet_type not in data_dict[num_rounds]:
                    data_dict[num_rounds][packet_type] = [timestamp]
                    data_dict[num_rounds]['Total Errors'] = len(data_dict[num_rounds]['ERR'])
                    error_pkts.append(data)
                else:
                    data_dict[num_rounds][packet_type].append(timestamp)
                    data_dict[num_rounds]['Total Errors'] = len(data_dict[num_rounds]['ERR'])
                    error_pkts.append(data)
                print("Error packets till now: ", error_pkts)
            
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
                print('Correct packets till now: ', correct_pkts)

            elif packet_type == 'nid':
                print(packet_type, data)
                data_dict, error_pkts, correct_pkts = get_error_positions(error_pkts, correct_pkts, data_dict, num_rounds)
                correct_pkts = []
                error_pkts = []
                correct_pkt_received = False

            else:
                if data == 'ROUND START':
                    #This part limits the size of the data frame to 1000 rows and writes it to a csv file
                    #Reset the data frame and start appending data to it again
                    print('Length of dataframe till now: ', len(data_frame))
                    if len(data_frame) == 200:
                        write_to_csv_file(data_frame, idx)
                        data_frame = pd.DataFrame(columns=['Round Number','Total Errors', 'ERR', 'OK!', 'FAIL!', 'BV_COUNT', 'ERR_POSITIONS'])
                        idx += 1
                    num_rounds += 1
                    print('--------------- ', data, ' : ',num_rounds,' ---------------')
                    data_dict[num_rounds] = {'Round Number': num_rounds, 'Total Errors': 0, 'ERR': [], 'OK!': 0, 'FAIL!': 0, 'BV_COUNT': [], 'ERR_POSITIONS': []}
                
                elif data == 'ROUND END':
                    if num_rounds not in data_dict:
                        data_dict[num_rounds] = {'Round Number': num_rounds, 'Total Errors': 0, 'ERR': [], 'OK!': 0, 'FAIL!': 0, 'BV_COUNT': [], 'ERR_POSITIONS': []}           
                    
                    data_dict, error_pkts, correct_pkts = get_error_positions(error_pkts, correct_pkts, data_dict, num_rounds)
                    
                    print('Writing to dataframe number: ', dataframe_number, " for round: ", num_rounds)
                    print("Dataframe:\n",data_dict[num_rounds])
                    new_df = pd.DataFrame([data_dict[num_rounds]])
                    data_frame = pd.concat([data_frame, new_df], ignore_index=True)
                    dataframe_number += 1
                    print('--------------- ',data,' : ', num_rounds, ' ---------------')
            print('--------------------------------------------------')
    idx = 0
    write_to_csv_file(data_frame, idx)
    return num_rounds, data_dict, error_pkts, correct_pkts, correct_pkt_received, data_frame, dataframe_number
    # (For Debug purposes)
    # write_to_csv_file(data_frame, idx)     

if __name__ =='__main__':
    num_rounds = 0
    data_dict = {}
    error_pkts = []
    correct_pkts = []
    correct_pkt_received = False
    dataframe_number = 0
    data_frame = pd.DataFrame(columns=['Round Number','Total Errors', 'ERR', 'OK!', 'FAIL!', 'BV_COUNT', 'ERR_POSITIONS'])
    # directory = './Output/'
    # for filename in os.listdir(directory):
    #     # Construct the full file path
    #     file_path = os.path.join(directory, filename)
    #     print('------------Opening file: ', file_path, ' ------------')
    #     # Check if the path is a file
    #     if os.path.isfile(file_path):
    log_strings = read_complete_file('./temp_log_file.txt')
    num_rounds, data_dict, error_pkts, correct_pkts, correct_pkt_received, data_frame, dataframe_number = parsing_logic(log_strings, num_rounds, data_dict, error_pkts, correct_pkts, correct_pkt_received, data_frame, dataframe_number)