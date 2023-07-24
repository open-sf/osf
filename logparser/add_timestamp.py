import re
import datetime
import os
import sys
import subprocess

# usage: python3 add_timestamp.py <window_length> <file_name>

class AddTS:

    def __init__(self, window_length,lg_dir) -> None:
        self.window_length = window_length
        self.current_time = datetime.datetime.now()
        self.time_delta = self.current_time + datetime.timedelta(minutes=self.window_length)
        self.logs_dir = lg_dir

    def create_log_folder(self):
        """
        This function creates a folder to store log files with the current timestamp
        :return: path of the log folder
        """
        curr_time = datetime.datetime.now()
        folder_name = curr_time.strftime("%Y%m%d_%H%M")

        log_folder_path = os.path.join(self.logs_dir, folder_name)

        if not os.path.exists(log_folder_path):
            print('Creating log folder: ', log_folder_path)
            os.makedirs(log_folder_path)

        return log_folder_path

    def is_next_window(self):
        """
        This function checks if the half and hour has passed since the last time it was called
        :return: True if half hour has passed, False otherwise
        """
        self.current_time = datetime.datetime.now()

        return self.current_time >= self.time_delta

    def reset_time_delta(self):
        self.time_delta = datetime.datetime.now() + datetime.timedelta(minutes=self.window_length)

    def add_timestamp(self, file_path):
        """
        This function adds timestamp to all the logs in the log file
        :param file_path: path of the log file
        :return: None
        """    
        #(For Windows)
        # powershell_command = f'Get-Content -Wait -Tail 1 "{file_path}"'
        # input_process = subprocess.Popen(['powershell.exe', '-Command', powershell_command], stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        
        # (For Linux) 
        input_process = subprocess.Popen(['tail', '-f', file_path], stdout=subprocess.PIPE)

        # Open the output file for writing
        log_folder = self.create_log_folder()
        output_file_name = log_folder+'/output_log'+datetime.datetime.now().strftime('%Y%m%d%H%M%S')+'.txt'
        output_file = open(output_file_name, 'w')   
        ansi_escape = re.compile(r'\x1B(?:[@-Z\\-_]|\[[0-?]*[ -/]*[@-~])')
        # Read and write the continuously updating input file
        while True:
            line = input_process.stdout.readline().decode().strip()
            line = ansi_escape.sub('', line)
                
            # Write the line to the output file
            to_be_written = '['+datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')+']'+" "+ line
            # print('-----------------------------')
            print(to_be_written)
            print('-----------------------------')
            output_file.write(to_be_written + '\n')
            output_file.flush()
            
            if self.is_next_window():
                output_file.close()
                print('-----------------------------Experiment Finished-----------------------------')
                input_process.terminate()
                # input_process.wait()
                return True,log_folder
        return None

def main(argv):
    if len(argv) != 2:
        print("please include a window length (min) and filename")
        print("usage: python3 add_timestamp.py <window_length> <file_name>")
        exit(1)

    try:
        float(argv[0])
    except:
        print("window length must be a number")
        print("usage: python3 add_timestamp.py <window_length> <file_name>")
        exit(1)

    if not os.path.exists('./'+argv[1]) and not os.path.exists(argv[1]):
        print("please enter a valid log file")
        print("usage: python3 add_timestamp.py <window_length> <file_name>")
        exit(1)

    ats_proc = AddTS(float(argv[0]))
    ats_proc.add_timestamp(argv[1])

# if __name__ == '__main__':
#     main(sys.argv[1:])