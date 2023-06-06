import re
import datetime
import pandas as pd
import time
import os
import io
import subprocess

def create_log_folder():
    """
    This function creates a folder to store log files with the current timestamp
    :return: path of the log folder
    """
    current_time = datetime.datetime.now()
    folder_name = current_time.strftime("%Y%m%d_%H%M")

    log_folder_path = os.path.join(os.getcwd(), "logs", folder_name)

    if not os.path.exists(log_folder_path):
        print('Creating log folder: ', log_folder_path)
        os.makedirs(log_folder_path)

    return log_folder_path

def is_next_half_hour():
    """
    This function checks if the half and hour has passed since the last time it was called
    :return: True if half hour has passed, False otherwise
    """

    current_time = datetime.datetime.now()
    next_half_hour = current_time + datetime.timedelta(minutes=1)

    return current_time >= next_half_hour

def add_timestamp(file_path):
    """
    This function adds timestamp to all the logs in the log file
    :param file_path: path of the log file
    :return: None

    """    
    line_number = 0

    powershell_command = f'Get-Content -Wait -Tail 1 "{file_path}"'
    input_process = subprocess.Popen(['powershell.exe', '-Command', powershell_command], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    #(For Linux) input_process = subprocess.Popen(['tail', '-f', input_file], stdout=subprocess.PIPE)

    # Open the output file for writing
    log_folder = create_log_folder()
    output_file = open(log_folder+'/output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt', 'w')
    ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
    
    # Read and write the continuously updating input file
    while True:
        line = input_process.stdout.readline().decode().strip()
        line = ansi_escape.sub('', line)
        line_number += 1
            
        # Write the line to the output file
        to_be_written = '['+datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')+']'+" "+ line
        print('-----------------------------')
        print(to_be_written)
        print('-----------------------------')
        output_file.write(to_be_written + '\n')
        output_file.flush()
        
        if line_number == 1000:
            output_file.close()
            output_file = open(log_folder+'output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt', 'w')
            line_number = 0
        if is_next_half_hour():
            output_file.close()
            log_folder = create_log_folder()
            output_file = open(log_folder+'output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt', 'w')
            line_number = 0
if __name__ == '__main__':
    # (For Debug purposes)
    # parse_file('./temp_log_file.txt') 
    add_timestamp('D:/Thesis Research/DataCollection/Logs/putty_dst.log')