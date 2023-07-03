import matplotlib.pyplot as plt
import pandas as pd
import datetime
import seaborn as sns
import os
import sys

# usage: python3 Plotting.py <log_dir>

class FileReader:
    """
    This class reads the files in the directory and stores the file paths in a dictionary
    """
    def __init__(self, dp) -> None:
        self.directory_path = dp
        self.filedirectory = {}
        self.window_number = 0
    
    def get_directories(self):
        """
        This function gets the directories in the Logoutput folder and stores them in a dictionary
        :return: None
        """
        # Specify the directory path
        subdirs = sorted(os.listdir(self.directory_path))
        # Iterate over the subdirectories
        for name in subdirs:
            if os.path.isdir(os.path.join(self.directory_path, name)):
                print('------------Adding Directory: ', os.path.join(self.directory_path, name), ' ------------')
                self.filedirectory[os.path.join(self.directory_path, name)] = []
    
    def read_files(self):
        """
        This function reads the files in the directory and stores the file paths in a dictionary
        :return: None
        """
        self.get_directories()
        for dir in self.filedirectory.keys():
            if os.path.isdir(dir+'/CSVFiles'):
                file_names = os.listdir(dir+'/CSVFiles')

                # Sort the file names based on modification time
                sorted_file_names = sorted(file_names, key=lambda x: os.path.getmtime(os.path.join(dir+'/CSVFiles/', x)))
                # Read the files in the sorted order
                for filename in sorted_file_names:
                    # Construct the full file path
                    file_path = os.path.join(dir+'/CSVFiles/', filename)
                    print('------------Adding file: ', file_path, ' ------------')
                    # Check if the path is a file
                    if os.path.isfile(file_path):
                        self.filedirectory[dir].append(file_path)

        return self.filedirectory


class DataPlotter:
    """ 
    This class plots different properties of data from the CSV files
    """
    def __init__(self, dp):
        self.error_positions = {}
        self.corrected_error_positions = {}
        self.tot_errs = {}
        self.tot_errs_time = {}
        self.window_number = 0
        self.fr = FileReader(dp)
        self.file_paths = {}
        # Colour-blind cycle by thriveth: https://gist.github.com/thriveth
        self.CB_color_cycle = ['#377eb8', '#ff7f00', '#4daf4a',
                                '#f781bf', '#a65628', '#984ea3',
                                '#999999', '#e41a1c', '#dede00']
    
    def get_all_file_paths(self):
        """
        This function gets the file paths dictionary from the FileReader class
        :return: None
        """
        self.file_paths = self.fr.read_files()
    
    def reset_window_number(self):
        """
        This function resets the window number to 0
        :return: None
        """
        self.window_number = 0

    def get_avg_rx_before_correction(self):
        """
        This function calculates the average number of receptions taken to
        successfully correct an error.
        :return: None
        """
        for filenames in self.file_paths.values():
            total_bv_count = 0
            num_oks = 0
            avg_bv_count = 0
            for filename in filenames:
                data = pd.read_csv(filename)
                for (bv_count, ok) in zip(data['BV_COUNT'], data['BV_SUCCESS_FLAG']):
                    bv_count = int(bv_count)

                    if ok == 1:
                        total_bv_count += bv_count
                        num_oks += 1
        if num_oks > 0:
            avg_bv_count = total_bv_count/num_oks
            print('Avg. Rx before correction: ', avg_bv_count)
        else:
            print('No successful corrections')
    
    def get_total_errors(self):
        """
        This function calculates the total errors from the CSV files. 
        The error is calculated as the sum of the total errors in each window
        The window size is 30 mintues. 
        The total errors are stored in a dictionary with the window number(Number of half hours passed) 
        as the key and the total errors as the value
        :return: None
        """
        for filenames in self.file_paths.values():
            for filename in filenames:
                data = pd.read_csv(filename)
                for err in data['N_ERR_PKTS']:
                    err = int(err)
                    if self.window_number not in self.tot_errs:
                        self.tot_errs[self.window_number] = err
                    else:
                        self.tot_errs[self.window_number] += err
                print('Total errors: ', self.tot_errs[self.window_number])
                self.window_number += 1

    def get_total_errors_time(self):
        """
        TO DO: Finish this
        
        This function calculates the total errors from the CSV files. 
        The error is calculated as the sum of the total errors in a
        certain time period.
        The total errors are stored in a dictionary with the window
        as the key and the total errors as the value
        :return: None
        """
        for filenames in self.file_paths.values():
            for filename in filenames:
                data = pd.read_csv(filename)
                window = str(curr_start_time.hour)+" - "+str(delta.hour)
                for (err, time) in zip(data['N_ERR_PKTS'], data['TIMESTAMP']):
                    time = datetime.datetime.strptime(time, "[%Y-%m-%d %H:%M:%S]")
                    if time < delta:
                        err = int(err)
                        if window not in self.tot_errs_time:
                            self.tot_errs_time[window] = err
                        else:
                            self.tot_errs_time[window] += err
                    else:
                        print('Total errors during ',window, ': ', self.tot_errs_time[window])
                        curr_start_time = time
                        delta = curr_start_time + datetime.timedelta(minutes=60)
                        window = str(curr_start_time.hour)+" - "+str(delta.hour)

    def plot_total_errors(self):
        """
        This function plots the total errors from the CSV files.
        :return: None
        """
        self.get_total_errors()

        # Sort the data dictionary by key
        sorted_data = sorted(self.tot_errs.items())
        df = pd.DataFrame(sorted_data, columns=['Window_Number', 'Num_Errs'])

        # Extract the sorted values and frequencies
        values, frequencies = zip(*sorted_data)

        # Set up the figure and axes
        plt.figure(figsize=(10, 6))
        # fig, ax = plt.subplots()
        sns.set_palette("colorblind")

        # Create the plot
        sns.scatterplot(df, x='Window_Number', y='Num_Errs', color=self.CB_color_cycle[0])

        # Set labels and title
        plt.xlabel('Time Window')
        plt.ylabel('Total Errors')
        plt.title('Total Errors Over Time')
        
        # Adjust x-axis tick labels and rotation
        plt.xticks(rotation=45)

        # Display the plot
        plt.show()

    def get_error_positions(self):
        """
        This function calculates the error positions from the CSV files.
        The error positions are stored in a dictionary with the error position as the key 
        and the frequency of error at that bit as the value
        :return: None
        """
        # Read the data from the csv file
        for filenames in self.file_paths.values():
            for filename in filenames:
                data = pd.read_csv(filename)
                # Extract the error positions
                for (err_poses, ok) in zip(data["ERRORS"], data["BV_SUCCESS_FLAG"]):
                    indices = err_poses.strip("\{\}").split(";")
                    if len(indices) > 0:
                        for index in indices:
                            pair = index.split(":")
                            if len(pair) > 1:
                                i = int(pair[0])
                                freq = int(pair[1])
                                if i not in self.error_positions:
                                    self.error_positions[i] = freq
                                else:
                                    self.error_positions[i] += freq

                                if ok == 1 and i not in self.corrected_error_positions:
                                    self.corrected_error_positions[i] = freq
                                elif ok == 1 and index in self.corrected_error_positions:
                                    self.corrected_error_positions[index] += freq

    def plot_error_positions(self):
        """
        This function plots the error positions from the CSV files.
        :return: None
        """
        # Extract error positions and frequencies
        self.get_error_positions()

        # Sort the data dictionary by key
        sorted_data = sorted(self.error_positions.items())

        # Extract the sorted values and frequencies
        values, frequencies = zip(*sorted_data)

        # Set up the figure and axes
        fig, ax = plt.subplots()

        # Create the bar plot
        ax.bar(values, frequencies, color=self.CB_color_cycle[0], label="No. Errors")

        # Add corrected-errors overlay if corrections occurred
        if len(self.corrected_error_positions) > 0:
            sorted_corrections = sorted(self.corrected_error_positions.items())
            corrected_vals, corrected_freqs = zip(*sorted_corrections)
            ax.bar(corrected_vals, corrected_freqs, color=self.CB_color_cycle[1], label="No. Corrections")

        # Set labels and title
        ax.set_xlabel('Bit Index in Packet')
        ax.set_ylabel('No. of Errors and Corrections')
        ax.set_title('Bit Errors vs Bit Corrections per Index')
        ax.legend()

        # Remove spines
        ax.spines['top'].set_visible(False)
        ax.spines['right'].set_visible(False)

        # Adjust x-axis tick labels rotation
        plt.xticks(rotation=45)

        # Display the plot
        plt.show()

def main(argv):
    if len(argv) < 1:
        print("usage: python3 Plotting.py <log_dir>")
        exit(1)
    elif not os.path.exists(argv[0]):
        print("please enter a valid directory")
        print("usage: python3 Plotting.py <log_dir>")
        exit(1)

    data_plotter = DataPlotter(argv[0])
    data_plotter.get_all_file_paths()
    data_plotter.plot_error_positions()
    # data_plotter.plot_total_errors()
    # data_plotter.get_avg_rx_before_correction()
    # data_plotter.get_total_errors_time()

if __name__ == '__main__':
    main(sys.argv[1:])