import re
import datetime
import pandas as pd
import time
import os
import io
import subprocess

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
    output_file = open('./Logoutput/output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt', 'w')
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
            output_file = open('./Logoutput/output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt', 'w')
            line_number = 0

if __name__ == '__main__':
    # (For Debug purposes)
    # parse_file('./temp_log_file.txt') 
    add_timestamp('D:/Thesis Research/DataCollection/Logs/putty_dst.log')