import matplotlib.pyplot as plt
import pandas as pd
import datetime
import seaborn as sns
import os
import sys
from read_log_files import FileReader
import datetime


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
        self.phys_lyr = ''
        self.fr = FileReader(dp)
        self.logs_dir = dp
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
        self.file_paths = self.fr.get_csv_file_paths()
    
    def set_physical_layer(self, pl):
        """
        This function sets the physical layer
        :return: None
        """
        self.phys_lyr = pl
    
    def reset_window_number(self):
        """
        This function resets the window number to 0
        :return: None
        """
        self.window_number = 0
    
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
                for (err_poses, ok, rnd) in zip(data["ERRORS"], data["BV_SUCCESS_FLAG"], data["ROUND"]):
                    indices = err_poses.strip("\{\}").split(";")
                    if len(indices) > 0:
                        for index in indices:
                            pair = index.split(":")
                            if len(pair) > 1:
                                i = int(pair[0])

                                # stop in case of errors
                                try:
                                    freq = int(pair[1])
                                except:
                                    print("err in rnd ", rnd, " index ", i)
                                    exit()

                                if(i > 2040 or freq > 6):
                                    print("err in rnd ", rnd, " index ", i)
                                    exit()

                                if i not in self.error_positions:
                                    self.error_positions[i] = freq
                                else:
                                    self.error_positions[i] += freq

                                if ok == 1 and i not in self.corrected_error_positions:
                                    self.corrected_error_positions[i] = freq
                                elif ok == 1 and i in self.corrected_error_positions:
                                    self.corrected_error_positions[i] += freq

    def plot_error_positions(self):
        """
        This function plots the error positions from the CSV files.
        :return: None
        """
        # Extract error positions and frequencies
        self.get_error_positions()

        # Sort the data dictionary by key
        sorted_data = sorted(self.error_positions.items())
        name_of_fig = self.logs_dir+"/Graphs/"+self.phys_lyr + "_error_positions"+datetime.datetime.now().strftime("%Y%m%d_%H%M")+".png"
        # Extract the sorted values and frequencies
        if len(sorted_data) > 0:
            values, frequencies = zip(*sorted_data)

        # Set up the figure and axes
        fig, ax = plt.subplots()

        # Create the bar plot
        if len(sorted_data) > 0:
            ax.bar(values, frequencies, color=self.CB_color_cycle[0], label="No. Errors")

            # Add corrected-errors overlay if corrections occurred
            if len(self.corrected_error_positions) > 0:
                sorted_corrections = sorted(self.corrected_error_positions.items())
                corrected_vals, corrected_freqs = zip(*sorted_corrections)
                ax.bar(corrected_vals, corrected_freqs, color=self.CB_color_cycle[1], label="No. Corrections")

            # Set labels and title
            ax.set_xlabel('Bit Index in Packet')
            ax.set_ylabel('No. of Errors and Corrections')
            ax.set_title(f'Bit Errors vs Bit Corrections per Index:{self.phys_lyr}, packet_length:{"128" if self.phys_lyr == "PHY_IEEE" else "255"}')
            ax.legend()

            # Remove spines
            ax.spines['top'].set_visible(False)
            ax.spines['right'].set_visible(False)

            # Adjust x-axis tick labels rotation
            plt.xticks(rotation=45)

            # Display the plot
            # plt.show()
            plt.savefig(name_of_fig)
            print('Saved plot to ', name_of_fig)
        else:
            
            print("No errors found")

def main(argv):
    # if len(argv) < 1:
    #     print("usage: python3 Plotting.py <log_dir>")
    #     exit(1)
    # elif not os.path.exists(argv[0]):
    #     print("please enter a valid directory")
    #     print("usage: python3 Plotting.py <log_dir>")
    #     exit(1)

    data_plotter = DataPlotter(argv[0])
    data_plotter.get_all_file_paths()
    data_plotter.plot_error_positions()
    # data_plotter.plot_total_errors()
    data_plotter.get_avg_rx_before_correction()
    # data_plotter.get_total_errors_time()
    data_plotter.get_err_pkts_correct_pkt()
# if __name__ == '__main__':
#     main(sys.argv[1:])