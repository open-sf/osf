import re
import datetime
import pandas as pd
import time
import os
import io
import subprocess

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

# def follow(filepath):
#     """
#     This function tails the log file for new logs and yeilds the new log
#     :param filepath: path of the log file
#     :return: new data from the log file
#     """

#     # Create a subprocess to continuously read updates from the log file
#     powershell_command = f'Get-Content -Wait -Tail 1 "{filepath}"'
#     tail_process = subprocess.Popen(['powershell.exe', '-Command', powershell_command], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)

#     # Continuously read and parse the log file updates
#     while True:
#         # Read the new data from the pipe
#         new_log = tail_process.stdout.readline().decode().strip()

#         # If no new data is available, wait for a short period and continue
#         if not new_log:
#             time.sleep(0.2)
#             continue
#         yield new_log

def add_timestamp(file_path):
    """
    This function adds timestamp to all the logs in the log file
    :param file_path: path of the log file
    :return: None

    """    
    arr_log_strings = []
    idx = 0

    powershell_command = f'Get-Content -Wait -Tail 1 "{file_path}"'
    input_process = subprocess.Popen(['powershell.exe', '-Command', powershell_command], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    #(For Linux) input_process = subprocess.Popen(['tail', '-f', input_file], stdout=subprocess.PIPE)

    # Open the output file for writing
    output_file = open('./Logoutput/output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt', 'w')
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    
    # Read and write the continuously updating input file
    while True:
        line = input_process.stdout.readline().decode().strip()
        line = ansi_escape.sub('', line)
        arr_log_strings.append(line)
        # Check if the input process has terminated
        # if line == '' and input_process.poll() is not None:
            
        # Write the line to the output file
        to_be_written = '['+datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')+']'+" "+ line
        print('-----------------------------')
        print(to_be_written)
        print('-----------------------------')
        output_file.write(to_be_written + '\n')
        output_file.flush()
        if len(arr_log_strings) == 1000:
            output_file.close()
            idx += 1
            output_file = open('./Logoutput/output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt', 'w')
            arr_log_strings = []

    



if __name__ == '__main__':
    # (For Debug purposes)
    # parse_file('./temp_log_file.txt') 
    add_timestamp('D:/Thesis Research/DataCollection/Logs/putty_dst.log')