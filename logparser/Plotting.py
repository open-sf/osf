import matplotlib.pyplot as plt
import pandas as pd
import datetime
import seaborn as sns
import os

class FileReader:
    """
    This class reads the files in the directory and stores the file paths in a dictionary

    """
    def __init__(self) -> None:
        self.filedirectory = {}
        self.window_number = 0
    
    def get_directories(self):
        """
        This function gets the directories in the Logoutput folder and stores them in a dictionary
        :return: None
        """
        # Specify the directory path
        directory_path = "./Logoutput/"

        # Iterate over the subdirectories
        for name in os.listdir(directory_path):
            if os.path.isdir(os.path.join(directory_path, name)):
                print('------------Adding Directory: ', os.path.join(directory_path, name), ' ------------')
                self.filedirectory[os.path.join(directory_path, name)] = []
    
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

    def get_directory_paths(self):
        """ 
        This function returns the dictionary containing the directory paths
        :return: dictionary containing the directory paths
        """
        
        self.read_files()
        return self.filedirectory

class DataPlotter:
    """ 
    This class plots different properties of data from the CSV files
    """
    def __init__(self):
        self.error_positions = {}
        self.tot_errs = {}
        self.window_number = 0
        self.fr = FileReader()
        self.file_paths = {}
    
    def get_all_file_paths(self):
        """
        This function gets the file paths dictionary from the FileReader class
        :return: None
        """
        self.file_paths = self.fr.get_directory_paths()
    
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
                for err in data['Total Errors']:
                    if err:
                        err = int(err)
                        if self.window_number not in self.tot_errs:
                            self.tot_errs[self.window_number] = err
                        else:
                            self.tot_errs[self.window_number] += err
            print('Total errors: ', self.tot_errs[self.window_number])
            self.window_number += 1

    def plot_total_errors(self):
        
        """
        This function plots the total errors from the CSV files.

        :return: None
        """
        
        
        self.get_total_errors()
        # Sort the data dictionary by key
        sorted_data = sorted(self.tot_errs.items())

        # Extract the sorted values and frequencies
        values, frequencies = zip(*sorted_data)

        # Set up the figure and axes
        fig, ax = plt.subplots()

        # Create the bar plot
        ax.bar(values, frequencies, color='steelblue')

        # Set labels and title
        ax.set_xlabel('Value')
        ax.set_ylabel('Frequency')
        ax.set_title('Frequency of Values')

        # Remove spines
        ax.spines['top'].set_visible(False)
        ax.spines['right'].set_visible(False)

        # Show gridlines
        ax.grid(axis='y', linestyle='--')

        # Adjust x-axis tick labels rotation
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
        
        # self.reset_window_number()

        # Read the data from the csv file
        for filenames in self.file_paths.values():
            for filename in filenames:
                data = pd.read_csv(filename)
                # Extract the error positions
                for indices in data['ERR_POSITIONS']:
                    indices = eval(indices)
                    # print(indices)
                    # print(type(indices))
                    if len(indices) > 0:
                        for index in indices:
                            if index not in self.error_positions:
                                self.error_positions[index] = 1
                            else:
                                self.error_positions[index] += 1
            self.window_number += 1

    def plot_error_positions(self):

        """
        This function plots the error positions from the CSV files.

        :return: None
        """
        # Extract error positions and frequencies
        self.get_error_positions()
        print(self.error_positions)
        # Sort the data dictionary by key
        sorted_data = sorted(self.error_positions.items())

        # Extract the sorted values and frequencies
        values, frequencies = zip(*sorted_data)

        # Set up the figure and axes
        fig, ax = plt.subplots()

        # Create the bar plot
        ax.bar(values, frequencies, color='steelblue')

        # Set labels and title
        ax.set_xlabel('Value')
        ax.set_ylabel('Frequency')
        ax.set_title('Frequency of Values')

        # Remove spines
        ax.spines['top'].set_visible(False)
        ax.spines['right'].set_visible(False)

        # Show gridlines
        ax.grid(axis='y', linestyle='--')

        # Adjust x-axis tick labels rotation
        plt.xticks(rotation=45)

        # Display the plot
        plt.show()

if __name__ == '__main__':
    data_plotter = DataPlotter()
    data_plotter.get_all_file_paths()
    data_plotter.plot_total_errors()
    data_plotter.plot_error_positions()