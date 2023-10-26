import matplotlib.pyplot as plt
import pandas as pd
import datetime
import seaborn as sns
import os
import sys
from read_log_files import FileReader
import datetime
import numpy as np


class DataPlotter:
    """ 
    This class plots different properties of data from the CSV files
    """
    def __init__(self, main_dp, dp, pkt_len, bv) -> None:
        self.error_positions = {}
        self.corrected_error_positions = {}
        self.phys_lyr = ''
        self.fr = FileReader(dp)
        self.logs_dir = main_dp
        self.file_paths = []
        # Colour-blind cycle by thriveth: https://gist.github.com/thriveth
        self.CB_color_cycle = ['#377eb8', '#ff7f00', '#4daf4a',
                                '#f781bf', '#a65628', '#984ea3',
                                '#999999', '#e41a1c', '#dede00']

        self.isbv = bv
        if self.isbv:
            self.pkt_len = pkt_len
        else:
            self.pkt_len = pkt_len
    
    def get_all_file_paths(self) -> None:
        """
        This function gets the file paths dictionary from the FileReader class
        :return: None
        """
        self.file_paths = self.fr.get_csv_file_paths()
    
    def set_physical_layer(self, pl) -> None:
        """
        This function sets the physical layer for which data plotting is being carried out
        :return: None
        """
        self.phys_lyr = pl
    
    def get_error_positions(self) -> None:
        """
        This function calculates the frequency of errors at each bit position from the CSV files.
        The error positions are stored in a dictionary with the error position as the key 
        and the frequency of error at that bit as the value
        :return: None
        """
        # Read the data from the csv file
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            # Extract the error positions
            for (err_poses, ok, rnd) in zip(data["ERRS"], data["BV_SCS_FLAG"], data["ROUND"]):
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
    
    def get_error_positions_no_bv(self) -> None:
        """
        This function calculates the error positions from the CSV files.
        The error positions are stored in a dictionary with the error position as the key 
        and the frequency of error at that bit as the value
        :return: None
        """
        # Read the data from the csv file
        for filename in self.file_paths:
            data = pd.read_csv(filename)
            # Extract the error positions
            for (err_poses, rnd) in zip(data["ERRS"], data["ROUND"]):
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

    def plot_error_positions(self, error_positions, plot_title, temperature=None, corrected_error_positions=None) -> None:
        """
        This function plots the error positions from the CSV files.
        :param error_positions: Dictionary of error positions and their frequencies
        :param plot_title: Title of the plot
        :param temperature: Temperature at which the data was collected
        :param corrected_error_positions: Dictionary of corrected error positions and their frequencies
        :return: None
        """
        # Extract error positions and frequencies
        if self.isbv:
            self.get_error_positions()
        else:
            self.get_error_positions_no_bv()

        # Sort the data dictionary by key
        sorted_data = sorted(error_positions.items())
        if temperature:
            name_of_fig = self.logs_dir+"/Graphs/"+self.phys_lyr + "_error_positions_"+datetime.datetime.now().strftime("%Y%m%d_%H%M")+"_"+temperature+".png"
        else:
            name_of_fig = self.logs_dir+"/Graphs/"+self.phys_lyr + "_error_positions_"+datetime.datetime.now().strftime("%Y%m%d_%H%M")+".png"
        # Extract the sorted values and frequencies
        if len(sorted_data) > 0:
            values, frequencies = zip(*sorted_data)

        # Set up the figure and axes
        fig, ax = plt.subplots()

        # Create the bar plot
        if len(sorted_data) > 0:
            ax.bar(values, frequencies, color=self.CB_color_cycle[0], label="No. Errors")

            # Add corrected-errors overlay if corrections occurred
            if len(corrected_error_positions) > 0:
                sorted_corrections = sorted(corrected_error_positions.items())
                corrected_vals, corrected_freqs = zip(*sorted_corrections)
                ax.bar(corrected_vals, corrected_freqs, color=self.CB_color_cycle[1], label="No. Corrections")

            # Set labels and title
            ax.set_xlabel('Bit Index in Packet')
            ax.set_ylabel('No. of Errors and Corrections')
            ax.set_title(plot_title)
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

    def plot_fft(self, sample_freq, power, cutoff_freq, temperature=None) -> None:
        """
        This function plots the FFT of the error positions from the CSV files.
        :param sample_freq: Sample frequency
        :param power: Power
        :param cutoff_freq: Cutoff frequency
        :return: None
        """

        # Plot the original signal and FFT result
        if len(sample_freq) > 0 and len(power) > 0:
            plt.figure(figsize=(10, 6))

            plt.plot(sample_freq, power, label='FFT')
            plt.xlabel("Frequency (Hz)")    
            plt.ylabel("Amplitude")
            plt.title("FFT Result")
            plt.xlim(1, cutoff_freq)

            plt.tight_layout()
            if not os.path.exists(self.logs_dir+"/Graphs/"):
                os.makedirs(self.logs_dir+"/Graphs/")
            
            if temperature:
                name_of_the_plot = self.logs_dir+"/Graphs/"+f'/fft_{self.phys_lyr}_'+datetime.datetime.now().strftime("%Y%m%d_%H%M")+"_"+temperature+".png"
            else:
                name_of_the_plot = self.logs_dir+"/Graphs/"+f'/fft_{self.phys_lyr}_'+datetime.datetime.now().strftime("%Y%m%d_%H%M")+".png"
            print('Saving FFT Figure as: ',name_of_the_plot)
            plt.savefig(name_of_the_plot, bbox_inches='tight')
        else:
            print('No FFT data to plot')
    
    
    def plot_beating_v_prr_pdr_all_phy(self, inp_dict, bv_mode, x_label, y_label, plt_title, stat_name) -> None:
        """
            This function plots the beating frequency vs PRR for all PHY layers
            :param inp_dict: Dictionary of beating frequency and PRR for all physical layers
            :param bv_mode: Bit voting mode
            :param x_label: X-axis label
            :param y_label: Y-axis label
            :param plt_title: Plot title
            :param stat_name: Name of the statistic that we want to plot against beating frequency
            :return: None
        """
        
        # Iterate through the dictionary and plot each line
        beating_frequency=[]
        stat_vals_each=[]
        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}

        for phy, phy_stats in inp_dict.items():
            print('------------------ PHY: ',phy,'--------------------')
            for stat_param, stat_vals in phy_stats.items():
                if len(stat_vals) > 0:
                    if stat_param == 'Beating Frequency':
                        beating_frequency = stat_vals
                    if stat_param == stat_name:
                        stat_vals_each = stat_vals
            print('BF: ',beating_frequency)
            print(stat_name,': ',stat_vals_each)
            plt.scatter(beating_frequency, stat_vals_each, label=phy+" "+bv_mode, marker=markers[phy])
            beating_frequency = []
            stat_vals_each = []

        # Set labels and title
        plt.xlabel(x_label)
        plt.ylabel(y_label)
        plt.title(plt_title)

        # Add a legend
        plt.legend()

        # Show the plot
        # plt.show()
        
    def plot_beating_v_prr_pdr_phy(self, inp_dict, phy, bv_mode, x_label, y_label, plt_title, stat_name) -> None:
        """
            This function plots the beating frequency vs PRR for a given PHY layer
            :param inp_dict: Dictionary of beating frequency and PRR
            :param phy: Physical Layer used
            :param bv_mode: Bit voting mode
            :param x_label: X-axis label
            :param y_label: Y-axis label
            :param plt_title: Plot title
            :param stat_name: Name of the statistic that we want to plot against beating frequency
            :return: None
        """

        # Iterate through the dictionary and plot each line
        beating_frequency=[]
        stat_vals_each=[]
        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}

        for key, val in inp_dict.items():
            if len(val) > 0:
                if key == 'Beating Frequency':
                    beating_frequency = val
                if key == stat_name:
                    stat_vals_each = val
        print('BF: ',beating_frequency)
        print(stat_name,': ',stat_vals_each)
        plt.scatter(beating_frequency, stat_vals_each, label=phy+" "+bv_mode, marker=markers[phy])

        # Set labels and title
        plt.xlabel(x_label)
        plt.ylabel(y_label)
        plt.title(plt_title)

        # Add a legend
        plt.legend()

        # Show the plot
        # plt.show()

    def plot_beating_v_total_corrections_phy(self, inp_dict, phy, bv_mode)-> None:
        """
            This function plots the beating frequency vs total corrections for a given PHY layer
            :param inp_dict: Dictionary of beating frequency and total corrections
            :param phy: Physical Layer used
            :param bv_mode: Bit voting mode
            :return: None
        """

        # Iterate through the dictionary and plot each line
        beating_frequency=[]
        corrections=[]
        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}

        for key, val in inp_dict.items():
            if len(val) > 0:
                if key == 'Beating Frequency':
                    beating_frequency = val
                if key == 'Corrections':
                    corrections = val
        print('BF: ',beating_frequency)
        print('Corrections: ',corrections)
        plt.scatter(beating_frequency, corrections, label=phy+bv_mode, marker=markers[phy])

        # Set labels and title
        plt.xlabel('Beating Frequency')
        plt.ylabel('Corrections')
        plt.title('Corrections vs Beating Frequency')

        # Add a legend
        plt.legend()
    
    def plot_prr_v_temperature_all_phy(self, inp_dict, bv_mode, xlabel, ylabel, plot_title) -> None:
        """
            This function plots the PRR vs Temperature for all PHY layers
            :param inp_dict: Dictionary of beating frequency and PRR for all physical layers
            :param bv_mode: Bit voting mode
            :param xlabel: X-axis label
            :param ylabel: Y-axis label
            :param plot_title: Plot title
            :return: None
        """
        
        # Iterate through the dictionary and plot each line

        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}

        if bv_mode:
            bv_mode = 'BV'
        else:
            bv_mode = 'No BV'

        for phy, phy_stats in inp_dict.items():
            print('------------------ PHY: ',phy,'--------------------')
            sorted_t = sorted(phy_stats['temperature_stat'].items())
            phy_stats = dict(sorted_t) 
            print('Sorted PHY Stats: ',phy_stats)
            plt.scatter(list(phy_stats.keys()), list(phy_stats.values()), label=phy+" "+bv_mode, marker=markers[phy])

        # Set labels and title
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)
        plt.title(plot_title)

        # Add a legend
        plt.legend()

        # Show the plot
        # plt.show()

    def plot_prr_v_temperature_one_phy(self, inp_dict, xlabel, ylabel, plot_title, physical_layer) -> None:
        """
            This function plots the PRR/PDR vs Temperature for one specific PHY layer
            :param inp_dict: Dictionary of beating frequency and PRR for all physical layers
            :param xlabel: X-axis label
            :param ylabel: Y-axis label
            :param plot_title: Plot title
            :param physical_layer: Physical layer
            :return: None
        """
        
        # Iterate through the dictionary and plot each line

        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}

        sorted_bv_stats = dict(sorted(inp_dict['BV']['temperature_stat'].items()))
        print('Sorted PHY Stats BV: ',sorted_bv_stats)
        sorted_no_bv_stats = dict(sorted(inp_dict['No_BV']['temperature_stat'].items()))
        print('Sorted PHY Stats No_BV: ',sorted_no_bv_stats)
        print('------------------ PHY: ',physical_layer,'--------------------')
        plt.scatter(list(sorted_bv_stats.keys()), list(sorted_bv_stats.values()), label=physical_layer+" BV", marker=markers[physical_layer]) 
        plt.scatter(list(sorted_no_bv_stats.keys()), list(sorted_no_bv_stats.values()), label=physical_layer+" "+"No_BV", marker=markers[physical_layer])

        # Set labels and title
        plt.xlabel(xlabel)
        plt.ylabel(ylabel)
        plt.title(plot_title)

        # Add a legend
        plt.legend()

        # Show the plot
        # plt.show()

    def plot_bf_v_temperature_all_phy(self, inp_dict, bv_mode, physical_layer=None) -> None:
        """
            This function plots the beating frequency vs temperature for all PHY layers
            :param inp_dict: Dictionary of beating frequency and PRR for all physical layers
            :param bv_mode: Bit voting mode
            :return: None
        """
        
        # Iterate through the dictionary and plot each line

        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}

        if bv_mode:
            bv_mode = 'BV'
        else:
            bv_mode = 'No BV'

        if not physical_layer:
            for phy, phy_stats in inp_dict.items():
                print('------------------ PHY: ',phy,'--------------------')
                sorted_t = sorted(phy_stats['temperature_bf'].items())
                phy_stats = dict(sorted_t) 
                print('Sorted PHY Stats: ',phy_stats)
                plt.scatter(list(phy_stats.keys()), list(phy_stats.values()), label=phy+" "+bv_mode, marker=markers[phy])
        else:
            sorted_t = sorted(inp_dict['temperature_bf'].items())
            phy_stats = dict(sorted_t) 
            print('Sorted PHY Stats: ',phy_stats)
            plt.scatter(list(phy_stats.keys()), list(phy_stats.values()), label=physical_layer+" "+bv_mode, marker=markers[physical_layer])
        # Set labels and title
        plt.yticks(np.arange(0,10001,500))
        plt.xlabel('Temperature')
        plt.ylabel('Beating Frequency')
        plt.title('Temperature vs Beating Frequency')

        # Add a legend
        plt.legend()

        # Show the plot
        # plt.show()
    
    def plot_corrections_v_temperature_all_phy(self, inp_dict, bv_mode) -> None:
        """
            This function plots the beating frequency vs temperature for all PHY layers
            :param inp_dict: Dictionary of beating frequency and PRR for all physical layers
            :param bv_mode: Bit voting mode
            :return: None
        """
        
        # Iterate through the dictionary and plot each line

        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}

        if bv_mode:
            bv_mode = 'BV'
        else:
            bv_mode = 'No BV'

        for phy, phy_stats in inp_dict.items():
            print('------------------ PHY: ',phy,'--------------------')
            sorted_t = sorted(phy_stats['temperature_corrections'].items())
            phy_stats = dict(sorted_t) 
            print('Sorted PHY Stats: ',phy_stats)
            plt.scatter(list(phy_stats.keys()), list(phy_stats.values()), label=phy+" "+bv_mode, marker=markers[phy])

        # Set labels and title
        plt.xlabel('Temperature')
        plt.ylabel('Corrections')
        plt.title('Temperature vs Corrections')

        # Add a legend
        plt.legend()

        # Show the plot
        # plt.show()

    def plot_prr_v_temperature_phy(self, inp_dict, x_label, y_label, plt_title, stat_name) -> None:
        """
            This function plots the PRR/PDR vs Beating frequency for all PHY layers for a specific temperature
            :param inp_dict: Dictionary of beating frequency and PRR for all physical layers
            :param bv_mode: Bit voting mode
            :param x_label: X-axis label
            :param y_label: Y-axis label
            :param plt_title: Plot title
            :return: None
        """
        
        # Iterate through the dictionary and plot each line

        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}
        # plt.figure(figsize=(5, 5))
        plt.subplots_adjust(right=0.6, top=0.9, bottom=0.1, left=0.1)
        for phy, phy_stats in inp_dict.items():
            print('------------------ PHY: ',phy,'--------------------')
            bv_data = phy_stats['BV']
            no_bv_data = phy_stats['No_BV']
            plt.scatter(bv_data['BF'], bv_data[stat_name], label=phy+" "+'BV', marker=markers[phy])
            plt.scatter(no_bv_data['BF'], no_bv_data[stat_name], label=phy+" "+'No_BV', marker=markers[phy])

        # Set labels and title
        plt.yticks(np.arange(0,101,5))
        # plt.xticks(np.arange(0,5001,500))
        plt.xlabel(x_label)
        plt.ylabel(y_label)
        plt.title(plt_title)

        # Add a legend
        plt.legend(loc='upper left', bbox_to_anchor=(1, 1))

        # Show the plot
        # plt.show()
    
    def plot_beating_v_per_phy(self, inp_dict, bv_mode, x_label, y_label, plt_title, stat_name,physical_layer=None, s=False, ) -> None:
        """
            This function plots the PER vs Beating frequency for all PHY layers for a specific temperature
            :param inp_dict: Dictionary of beating frequency and PRR for all physical layers
            :param bv_mode: Bit voting mode
            :param x_label: X-axis label
            :param y_label: Y-axis label
            :param plt_title: Plot title
            :param stat_name: Name of the statistic that we want to plot against beating frequency
            :param physical_layer: Physical layer
            :param s: A bool that says that dictionary has data for a single physical layer only
            :return: None
        """
        
        # Iterate through the dictionary and plot each line

        markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}
        # plt.figure(figsize=(5, 5))
        if not s:
            if not physical_layer:
                for phy, phy_stats in inp_dict.items():
                    print('------------------ PHY: ',phy,'--------------------')
                    plt.scatter(phy_stats['BF'], phy_stats[stat_name], label=phy+" "+bv_mode, marker=markers[phy])
            else:
                plt.scatter(inp_dict[physical_layer]['BF'], inp_dict[physical_layer][stat_name], label=physical_layer+" "+bv_mode, marker=markers[physical_layer])
        else:
            if inp_dict['BV']:
                plt.scatter(inp_dict['BV']['BF'], inp_dict['BV'][stat_name], label=physical_layer+" "+'BV', marker=markers[physical_layer])
            if inp_dict['No_BV']:
                plt.scatter(inp_dict['No_BV']['BF'], inp_dict['No_BV'][stat_name], label=physical_layer+" "+'No_BV', marker=markers[physical_layer])

        # Set labels and title
        # plt.yticks(np.arange(0,101,5))
        plt.xticks(np.arange(0,5001,500))
        plt.xlabel(x_label)
        plt.ylabel(y_label)
        plt.title(plt_title)

        # Add a legend
        plt.legend()
        # plt.legend(loc='upper left', bbox_to_anchor=(1, 1))

        # Show the plot
        # plt.show()