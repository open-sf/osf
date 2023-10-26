from typing import Any
import matplotlib.pyplot as plt
from read_log_files import FileReader
import pandas as pd
import json
from Plotting import DataPlotter
import numpy as np
import datetime
import argparse
import sys

def make_box_plot() -> None:
    """
        This function creates box plots for each physical layer.
        The plot is drawn for PRR values for each physical layer for the cases when BV is used and when BV is not used.
        The values are divided into 5 categories based on the packet size.
        :return: None
    """

    pkt_size_prr_vals_dict = {16:{
        '1M':{'BV':[91.33,39.74,20.77,90.41,41.84],
              'NO_BV':[86.44,85.9,7.96,85.11,14.56]},
        '2M':{'BV':[53.25,35.61,19.24,95.34,54.54],
              'NO_BV':[12.92,60.29,74.85,16.58,47.38],},
        '125K':{'BV':[71.35,67.79,54.05,83.33,71.89],
                'NO_BV':[73.51,92.30,67.7,76.31,63.1]},
        '500K':{'BV':[82.03,84.79,68.57,93.22,68.85],
                'NO_BV':[93.15,57.94,88.05,83.57,56.99]},
        'IEEE':{'BV':[69.94,100,89.2,81.08,67.36],
                'NO_BV':[73.36,80.11,69.51,72.92,79.16]}, 
    },
                              32:{
                                '1M':{'BV':[83.33,89.85,81.7,50.7,77.7],
                                      'NO_BV':[81.39,79.76,81.43,77.38,52.19]},
                                '2M':{'BV':[65.11,60.69,40.92,64.86,40.9],
                                      'NO_BV':[51.74,28.95,50,38.86,55.02]},
                                '125K':{'BV':[75.12,69.35,60.65,56.57,50],
                                        'NO_BV':[65.64,59.53,49.75,66.52,58.48]},
                                '500K':{'BV':[42.91,87.58,83.73,41.47,80.12],
                                        'NO_BV':[75.72,82.91,81.5,85.79,45.96]},
                                'IEEE':{'BV':[73.51,77.43,55.94,74.85,50.23],
                                          'NO_BV':[76.68,87.01,64.28,85.23,72.34]},
                                
                              },
                              64:{
                                '1M':{'BV':[71.28,70.98,66.81,69.13,67.06],
                                      'NO_BV':[57.39,45.55,74.40,71.83,64.13]},
                                '2M':{'BV':[37.03,30,27.62,73.48,22.97],
                                      'NO_BV':[33.46,4.65,48.7,2.97,28.85]},
                                '125K':{'BV':[40.75,45.93,42.36,38.98,71.6],
                                        'NO_BV':[33,51.4,41.84,40.64,40.4]},
                                '500K':{'BV':[59.56,72.25,71.3,60.38,75.65],
                                        'NO_BV':[73.18,59.92,51.39,67.77,56.95]},
                                'IEEE':{'BV':[44.48,64.34,59.06,68.68,60.89],
                                          'NO_BV':[50.89,55.41,42.47,70.61,46.64]},
                              },
                              125:{
                                '1M':{'BV':[58.76,60.09,54.14,60.91,47.88],
                                      'NO_BV':[50.44,46.88,44.67,55.17,46]},
                                '2M':{'BV':[26.55,15.98,30.79,15.5,32.58],
                                      'NO_BV':[27.4,2.9,35.94,5.86,15.2]},
                                '125K':{'BV':[18.55,28.44,15.38,45.11,21.23],
                                        'NO_BV':[14.21,49.35,9.61,22.58,16.9]},
                                '500K':{'BV':[43.16,45.74,40.25,57.48,49.49],
                                        'NO_BV':[41.53,56.83,43.94,50.86,50.42]},
                                'IEEE':{'BV':[23.62,34.71,43.63,42.7,32.58],
                                          'NO_BV':[25.5,37.14,30.31,34.07,22.01]},
                              },
                              255:{
                                '1M':{'BV':[47.36,38.65,32.69,52.87,25.4],
                                      'NO_BV':[34.72,29.81,45.28,38.7,19.43]},
                                '2M':{'BV':[27.51,36.25,27.62,15.87,14.99],
                                      'NO_BV':[24.92,2.73,16.26,33.33,36.23]},
                                '125K':{'BV':[8.87,16.63,11.71,36.9,15.08],
                                        'NO_BV':[3.86,13.51,1.86,3.83,4.07]},
                                '500K':{'BV':[34.81,37.37,25.41,41.69,25.25],
                                        'NO_BV':[30,11.32,24.81,26.54,19.67]},
                              }}
    packet_sizes = list(pkt_size_prr_vals_dict.keys())
    prr_values = {}

    for pkt_size, pkt_size_data in pkt_size_prr_vals_dict.items():
        for phy_layer, prr_data in pkt_size_data.items():
            for modulation, values in prr_data.items():
                key = (pkt_size, phy_layer, modulation)
                prr_values[key] = values
    # print(prr_values)
    # Create box plots for each packet size
    for pkt_size in packet_sizes:
        fig, ax = plt.subplots()
        ax.boxplot([prr_values[(pkt_size, phy_layer, 'BV')] for phy_layer in pkt_size_prr_vals_dict[pkt_size].keys()],
                labels=list(pkt_size_prr_vals_dict[pkt_size].keys()))
        
        ax.set_title(f'Box Plot for Packet Size {pkt_size}')
        ax.set_xlabel('Physical Layer')
        ax.set_ylabel('PRR Values (%)')
        
        plt.savefig(f'./Boxplots/box_plot_{pkt_size}.png')
    
    for pkt_size, pkt_size_data in pkt_size_prr_vals_dict.items():
        print(f"Packet Size: {pkt_size}")
        for phy_layer, prr_data in pkt_size_data.items():
            bv_average = sum(prr_data['BV']) / len(prr_data['BV'])
            nobv_average = sum(prr_data['NO_BV']) / len(prr_data['NO_BV'])
            print(f"Physical Layer: {phy_layer}")
            print(f"Average BV: {bv_average:.2f}")
            print(f"Average NO_BV: {nobv_average:.2f}")
        print()


class StatsPlotter:

	def __init__(self) -> None:
		self.bf_v_stat = {'PHY_BLE_2M':{
			'Beating Frequency':[]
		}, 'PHY_BLE_1M':{
			'Beating Frequency':[]
		}, 'PHY_BLE_500K':{
			'Beating Frequency':[]
		}, 'PHY_BLE_125K':{
			'Beating Frequency':[]
		}}

		self.bf_v_stats_phywise = {
			'No_BV':{
			'Beating Frequency':[]},
			'BV':{      
			'Beating Frequency':[]}
		}
		
		self.temperature_v_stats = {'PHY_BLE_2M':{
		'temperature_stat':{}
		}, 'PHY_BLE_1M':{
		'temperature_stat':{}
		}, 'PHY_BLE_500K':{
		'temperature_stat':{}
		}, 'PHY_BLE_125K':{
		'temperature_stat':{}
		}}

		self.temperature_v_stats_phywise = {
			'No_BV':{
			'temperature_stat':{}},
			'BV':{      
			'temperature_stat':{}}
		}

		self.phy_wise_stats = {
			'BV':{},
			'No_BV':{}
		}

		

	def print_dictionary(self, inp_dict):
		"""
				This function prints the dictionary in a readable format
				:param inp_dict: Dictionary to be printed
				:return: None
		"""

		formatted_dict = json.dumps(inp_dict, default=str, indent=4)
		print(formatted_dict)

	def plot_beating_vs_prr_pdr_all_phys(self, power, bv, stat_name, pair_wise=False, src=None, fwd=None, dst=None):
		"""
			This function plots the beating frequency vs PRR for all physical layers for a single mode of bit voting
			:param power: Power used for the experiment
			:param stat_name: Name of the statistic that we want to plot against beating frequency
			:param bv: Boolean value to indicate if bit voting was used or not
			:param pair_wise: Boolean value to indicate if the plotting is to be done for a specific node pair
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:return: None
		"""
		path = f'/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab/{power}'
		reader = FileReader(path)
		if not pair_wise:
			csv_files_dict = reader.get_dcube_stat_csv_files(bv)
		else:
			csv_files_dict = reader.get_dcube_csv_files_pair(bv, str(src), str(fwd), str(dst))
		# self.print_dictionary(csv_files_dict)

		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)
		for phy, phy_stat_csv_files in csv_files_dict.items():
			print(f"--------------- PHY: {phy} -----------------------")
			self.bf_v_stat[phy][stat_name] = []
			for csv_file in phy_stat_csv_files:
				print(f"Reading file {csv_file}")
				data = pd.read_csv(csv_file)
				if not data.empty:
					self.bf_v_stat[phy][stat_name].append(data[stat_name].values[0])
					self.bf_v_stat[phy]['Beating Frequency'].append(data['Beating_Frequency'].values[0])
				# self.print_dictionary(self.bf_v_stat[phy])
				print('--------------------------------------------------------')

		print('\n ---------------------- Complete Dictionary ----------------------------------- \n')
		self.print_dictionary(self.bf_v_stat)       
		print('\n --------------------------------------------------------- \n')
		if bv:
			bv_mode = 'with_bv'
		else:
			bv_mode = 'no_bv'
		
		x_l = 'Beating Frequency (Hz)'
		y_l = stat_name + " (%)"
		plot_title = f'Beating Frequency vs {stat_name}'
		plotter.plot_beating_v_prr_pdr_all_phy(self.bf_v_stat, bv_mode, x_l, y_l, plot_title, stat_name)
		plt.show()
	
	def plot_beating_v_prr_pdr_phywise(self, power, physical_layer, stat_name):
		"""
			This function plots the beating frequency vs PRR/PDR for a specific physical layer.
			:param power: Power used for the experiment
			:param physical_layer: Physical layer used for the experiment
			:param stat_name: Name of the statistic that we want to plot against beating frequency
			:return: None
		"""
		# path = f'/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab/{power}'
		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10/'
		reader = FileReader(path)


		csv_files_dict_bv = reader.get_dcube_csv_files_pair(True, str(122), str(126), str(124))
		reader_2 = FileReader(path)
		csv_files_dict_no_bv = reader_2.get_dcube_csv_files_pair(False, str(122), str(126), str(124))
		x_l = 'Beating Frequency (Hz)'
		y_l = stat_name + ' (%)'
		plot_title = f'Beating Frequency vs {stat_name}'
		self.bf_v_stats_phywise['BV'][stat_name] = []
		self.bf_v_stats_phywise['No_BV'][stat_name] = []
		print(f'-------------------------- {stat_name} ----------------------------')
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)
		for (csv_file_bv, csv_file_no_bv) in zip(csv_files_dict_bv[physical_layer], csv_files_dict_no_bv[physical_layer]):
			print('\n--------------------------------------------------------\n')
			print(f"Reading file {csv_file_bv}")
			print(f"Reading file {csv_file_no_bv}")
			data_bv = pd.read_csv(csv_file_bv)
			data_no_bv = pd.read_csv(csv_file_no_bv)
			if not data_bv.empty:
				self.bf_v_stats_phywise['BV'][stat_name].append(data_bv[stat_name].values[0])
				self.bf_v_stats_phywise['BV']['Beating Frequency'].append(data_bv['Beating_Frequency'].values[0])
			if not data_no_bv.empty:
				self.bf_v_stats_phywise['No_BV'][stat_name].append(data_no_bv[stat_name].values[0])
				self.bf_v_stats_phywise['No_BV']['Beating Frequency'].append(data_no_bv['Beating_Frequency'].values[0])
		
		plotter.plot_beating_v_prr_pdr_phy(self.bf_v_stats_phywise['BV'], physical_layer, 'BV', x_l, y_l, plot_title, stat_name)
		print(f"Standard Deviation {stat_name} BV: {str(np.std(self.bf_v_stats_phywise['BV'][stat_name]))}")
		print(f"Average {stat_name} BV {str(np.average(self.bf_v_stats_phywise['BV'][stat_name]))}")
		print('Standard Deviation BF BV: ', str(np.std(self.bf_v_stats_phywise['BV']['Beating Frequency'])))
		print('Average BF BV', str(np.average(self.bf_v_stats_phywise['BV']['Beating Frequency'])))

		plotter.plot_beating_v_prr_pdr_phy(self.bf_v_stats_phywise['No_BV'], physical_layer, 'No BV', x_l, y_l, plot_title, stat_name)
		print(f"Standard Deviation {stat_name} No_BV: {str(np.std(self.bf_v_stats_phywise['No_BV'][stat_name]))}")
		print(f"Average {stat_name} No_BV {str(np.average(self.bf_v_stats_phywise['No_BV'][stat_name]))}")
		print('Standard Deviation BF No_BV: ', str(np.std(self.bf_v_stats_phywise['No_BV']['Beating Frequency'])))
		print('Average BF No_BV:', str(np.average(self.bf_v_stats_phywise['No_BV']['Beating Frequency'])))
		print('--------------------------------------------------------')
		
		if stat_name == 'PRR':
			plt.savefig(f'./Graphs/Beating_vs_PRR_graphs_S=122_F=126_D=124/prr_vs_bf_{physical_layer}_{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}.png')
		elif stat_name == 'PDR':
			plt.savefig(f'./Graphs/Beating_vs_PDR_graphs_S=122_F=126_D=124/pdr_vs_bf_{physical_layer}_{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}.png')

		# plt.savefig(f'./Graphs/Beating_vs_PRR_graphs_S=122_F=126_D=124/beating_vs_prr_{physical_layer}_{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}.png')
	
	#Check the plotting function of this function
	def plot_beating_freq_vs_corrections_phywise(self, power, physical_layer):
		"""
			This function plots the beating frequency vs total corrections for a specific physical layer.
			:param power: Power used for the experiment
			:param physical_layer: Physical layer used for the experiment
			:return: None
		"""

		path = f'/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab/{power}'
		reader = FileReader(path)

		csv_files_dict_bv = reader.get_dcube_csv_files_pair(True, str(122), str(126), str(124))
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)
		self.bf_v_stats_phywise['BV']['Corrections'] = []
		for csv_file in csv_files_dict_bv[physical_layer]:
			print(f"Reading file {csv_file}")
			data = pd.read_csv(csv_file)
			if not data.empty:
					self.bf_v_stat['Corrections'].append(data['Total_Corrections'].values[0])
					self.bf_v_stat['Beating Frequency'].append(data['Beating_Frequency'].values[0])
		self.print_dictionary(self.bf_v_stat)
		plotter.plot_beating_v_total_corrections(self.bf_v_stats_phywise, physical_layer, 'BV')
		plt.savefig(f'./Boxplots/beating_vs_correctoins_{physical_layer}_{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}.png')

	def plot_average_corrections_v_bf_all_phys(self, power, pair_wise=False, src=None, fwd=None, dst=None):	
		"""
			This function plots the average corrections vs beating frequency for all physical layers
			:param pair_wise: Boolean value to indicate if the plotting is to be done for a specific node pair
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:return: None
		"""
		
		path = f'/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab/{power}'
		reader = FileReader(path)

		average_corrections = {'PHY_BLE_2M': 0, 'PHY_BLE_1M': 0, 'PHY_BLE_500K': 0, 'PHY_BLE_125K': 0}
		average_beating_frequency = {'PHY_BLE_2M': 0, 'PHY_BLE_1M': 0, 'PHY_BLE_500K': 0, 'PHY_BLE_125K': 0}

		csv_files_dict_bv = reader.get_dcube_csv_files_pair(True, str(src), str(fwd), str(dst))
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)
		for phy, phy_stat_csv_files in csv_files_dict_bv.items():
			print(f"--------------- PHY: {phy} -----------------------")
			self.bf_v_stat[phy]['Corrections'] = []
			for csv_file in phy_stat_csv_files:
					print(f"Reading file {csv_file}")
					data = pd.read_csv(csv_file)
					if not data.empty:
							self.bf_v_stat[phy]['Corrections'].append(data['Total_Corrections'].values[0])
							self.bf_v_stat[phy]['Beating Frequency'].append(data['Beating_Frequency'].values[0])
			average_corrections[phy] = np.average(self.bf_v_stat[phy]['Corrections'])
			average_beating_frequency[phy] = np.average(self.bf_v_stat[phy]['Beating Frequency'])
		
		print('------------------ Original Dictionary----------------------')
		self.print_dictionary(self.bf_v_stat)
		print('--------------- Average Correctoins -----------------------')
		self.print_dictionary(average_corrections)
		print('--------------- Average Beating Frequency -----------------------')
		self.print_dictionary(average_beating_frequency)
		markers = {'PHY_BLE_1M':'o', 'PHY_BLE_2M':'s', 'PHY_BLE_125K':'D', 'PHY_BLE_500K':'^'}
		plt.scatter(average_beating_frequency.values(), average_corrections.values(), marker='o')

		for label, x, y in zip(average_beating_frequency.keys(), average_beating_frequency.values(), average_corrections.values()):
				plt.annotate(label, xy=(x, y), xytext=(-20, 20), textcoords='offset points', ha='right', va='bottom',
										bbox=dict(boxstyle='round,pad=0.5', fc='yellow', alpha=0.5),
										arrowprops=dict(arrowstyle='->', connectionstyle='arc3,rad=0'))

		# plt.scatter(average_corrections.keys(), average_corrections.values(), marker='x')
		plt.xlabel('Average Beating Frequency')
		plt.ylabel('Average Corrections')
		plt.title('Average Corrections vs Average Beating Frequency')
		plt.show()
    
	def plot_avg_prr_v_temperature_all_phys(self, heating_node, bv_mode, src, fwd, dst, stat_name):
		"""
			This function plots the PDR/PRR vs temperature for all physical layers.
			Here the Average PRR for each temperature is considered
			:param heating_node: Node which is being heated
			:param bv_mode: Mode of bit voting
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:param stat_name: Name of the statistic that we want to plot against temperature
			:return: None
		"""

		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab'
		if heating_node == 'fwd':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Forwarder'
		elif heating_node == 'src':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10'

		reader = FileReader(path)

		x_label = 'Temperature (C)'
		y_label = stat_name + ' (%)'
		plot_title = f'{stat_name} vs Temperature'

		csv_files_dict = reader.get_dcube_csv_files_pair_temperature(bv_mode, str(src), str(fwd), str(dst))
		# self.print_dictionary(csv_files_dict)
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)

		for phy, phy_stat_temp_csv_files in csv_files_dict.items():
			print(f"--------------- PHY: {phy} -----------------------")
			auxiliary_prr = []
			for temperature, csv_files in phy_stat_temp_csv_files.items():
				print(f"--------------- Temperature: {temperature} -----------------------")
				for csv_file in csv_files:
					print(f"Reading file {csv_file}\n")
					data = pd.read_csv(csv_file)
					if not data.empty:
							auxiliary_prr.append(data[stat_name].values[0])
				temperature = int(temperature[:-1])
				if temperature not in self.temperature_v_stats[phy]['temperature_stat']:
					self.temperature_v_stats[phy]['temperature_stat'][temperature] = np.average(auxiliary_prr)
				else:
					self.temperature_v_stats[phy]['temperature_stat'][temperature] = np.average(auxiliary_prr)
				auxiliary_prr = []
		self.print_dictionary(self.temperature_v_stats)
		plotter.plot_prr_v_temperature_all_phy(self.temperature_v_stats, bv_mode, x_label, y_label, plot_title)
		
		plt.show()

	#Complete this function
	def plot_avg_prr_v_temperature_one_phy(self, heating_node, src, fwd, dst, stat_name, physical_layer):
		"""
			This function plots the PDR/PRR/PER vs temperature for a specific physical layer.
			Here the Average PRR/PDR for each temperature is considered
			:param heating_node: Node which is being heated
			:param bv_mode: Mode of bit voting
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:param stat_name: Name of the statistic that we want to plot against temperature
			:param physical_layer: Physical layer for which the plot is to be drawn
			:return: None
		"""

		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab'
		if heating_node == 'fwd':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Forwarder'
		elif heating_node == 'src':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10'

		reader = FileReader(path)

		x_label = 'Temperature (C)'
		y_label = stat_name + ' (%)'
		plot_title = f'{stat_name} vs Temperature'
		stat_name_t = None
		if stat_name == 'PER':
			stat_name = 'PRR'
			stat_name_t = 'PER'			

		csv_files_dict_bv = reader.get_dcube_csv_files_pair_temperature(True, str(src), str(fwd), str(dst))
		reader = FileReader(path)
		csv_files_dict_no_bv = reader.get_dcube_csv_files_pair_temperature(False, str(src), str(fwd), str(dst))
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)
		sorted_bv_files = dict(sorted(csv_files_dict_bv[physical_layer].items()))
		sorted_no_bv_files = dict(sorted(csv_files_dict_no_bv[physical_layer].items()))
		for (temperature_bv, csv_temp_files_bv), (temperature_no_bv, csv_temp_files_no_bv) in zip(sorted_bv_files.items(), sorted_no_bv_files.items()):
			auxiliary_prr_bv = []
			auxiliary_prr_no_bv = []
			print(f"--------------- Temperature: {temperature_bv }{temperature_no_bv} -----------------------")
			
			for (csv_file_bv, csv_file_no_bv) in zip(csv_temp_files_bv, csv_temp_files_no_bv):
				print(f"Reading file {csv_file_bv}\n")
				print(f"Reading file {csv_file_no_bv}\n")
				data_bv = pd.read_csv(csv_file_bv)
				data_no_bv = pd.read_csv(csv_file_no_bv)
				if not data_bv.empty:
						auxiliary_prr_bv.append(data_bv[stat_name].values[0])
				if not data_no_bv.empty:
						auxiliary_prr_no_bv.append(data_no_bv[stat_name].values[0])

			temperature = int(temperature_no_bv[:-1])
			if temperature not in self.temperature_v_stats_phywise['BV']['temperature_stat']:
				self.temperature_v_stats_phywise['BV']['temperature_stat'][temperature] = np.average(auxiliary_prr_bv)
				if stat_name_t is not None and stat_name_t == 'PER':
					self.temperature_v_stats_phywise['BV']['temperature_stat'][temperature] = 100 - np.average(auxiliary_prr_bv)
			
			if temperature not in self.temperature_v_stats_phywise['No_BV']['temperature_stat']:
				self.temperature_v_stats_phywise['No_BV']['temperature_stat'][temperature] = np.average(auxiliary_prr_no_bv)
				if stat_name_t is not None and stat_name_t == 'PER':
					self.temperature_v_stats_phywise['No_BV']['temperature_stat'][temperature] = 100 - np.average(auxiliary_prr_no_bv)
			print('BV: ', auxiliary_prr_bv)
			print('No BV: ', auxiliary_prr_no_bv)
			auxiliary_prr_bv = []
			auxiliary_prr_no_bv = []
		self.print_dictionary(self.temperature_v_stats_phywise)
		plotter.plot_prr_v_temperature_one_phy(self.temperature_v_stats_phywise, x_label, y_label, plot_title, physical_layer)
		if stat_name_t is not None and stat_name_t == 'PER':
			stat_name = 'PER'
		fig_save_path = '/home/burhanuddin/Desktop/osf/logparser/Graphs/Increasing_Temperature/'+physical_layer+'/'+stat_name+'_vs_Temp/'+f'plot_{stat_name}_vs_Temp_{physical_layer}_{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}.png'
		# plt.savefig(fig_save_path)
		# plt.show()

	def plot_avg_prr_v_bf_temperature_all_phys(self, heating_node, src, fwd, dst, temperature, stat_name):
		"""
			This function plots the PRR/PDR vs Beating Frequency for all physical layers for a specific temperature.
			Here the Average PRR/PDR for each temperature is considered
			:param heating_node: Node which is being heated
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:param temperature: Temperature for which the plot is to be drawn
			:param stat_name: Name of the statistic that we want to plot against temperature
			:return: None
		"""

		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab'
		if heating_node == 'fwd':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Forwarder'
		elif heating_node == 'src':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10'

		reader = FileReader(path)
		temperature_stats = {'PHY_BLE_2M':{
			'BV':{
				'BF':[],
			},
			'No_BV':{
				'BF':[],
			}

		}, 'PHY_BLE_1M':{
			'BV':{
				'BF':[],
			},
			'No_BV':{
				'BF':[],
			}

		}, 'PHY_BLE_500K':{
			'BV':{
				'BF':[],
			},
			'No_BV':{
				'BF':[],
			}
		}, 'PHY_BLE_125K':{
			'BV':{
				'BF':[],
			},
			'No_BV':{
				'BF':[],
			}
		}}
		csv_files_dict_bv = reader.get_dcube_csv_files_pair_temperature(True, str(src), str(fwd), str(dst))
		reader_2 = FileReader(path)
		csv_files_dict_no_bv = reader_2.get_dcube_csv_files_pair_temperature(False, str(src), str(fwd), str(dst))
		# self.print_dictionary(csv_files_dict_bv)
		# self.print_dictionary(csv_files_dict_no_bv)
		# sys.exit(1)
		stat_name_t = None
		if stat_name == 'PER':
			stat_name = 'PRR'
			stat_name_t = 'PER'
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)

		#BV
		print('--------------BV------------------')
		for (phy, phy_stat_bv_csv_files),(phy_no_bv, phy_stat_no_bv_csv_files) in zip(csv_files_dict_bv.items(), csv_files_dict_no_bv.items()):
			print(f"--------------- PHY: {phy} {phy_no_bv}-----------------------")
			auxiliary_stat_bv = []
			auxiliary_bf_bv = []
			auxiliary_stat_no_bv = []
			auxiliary_bf_no_bv = []
			# print(phy_stat_csv_files[temperature])
			print(f"--------------- Temperature: {temperature} -----------------------")
			# sys.exit(1)
			for (csv_file_bv, csv_file_no_bv) in zip(phy_stat_bv_csv_files[temperature], phy_stat_no_bv_csv_files[temperature]):
				print(f"Reading file {csv_file_bv}\n")
				print(f"Reading file {csv_file_no_bv}\n")
				data_bv = pd.read_csv(csv_file_bv)
				data_no_bv = pd.read_csv(csv_file_no_bv)
				if not data_bv.empty:
						if stat_name_t is not None and stat_name_t == 'PER':
							auxiliary_stat_bv.append(100 - data_bv[stat_name].values[0])
						else:
							auxiliary_stat_bv.append(data_bv[stat_name].values[0])
						auxiliary_bf_bv.append(data_bv['Beating_Frequency'].values[0])
				if not data_no_bv.empty:
						if stat_name_t is not None and stat_name_t == 'PER':
							auxiliary_stat_no_bv.append(100 - data_no_bv[stat_name].values[0])
						else:
							auxiliary_stat_no_bv.append(data_no_bv[stat_name].values[0])
						auxiliary_bf_no_bv.append(data_no_bv['Beating_Frequency'].values[0])
			print('---------BV---------\n')
			print(auxiliary_stat_bv)
			print(auxiliary_bf_bv)
			print('---------NO BV---------\n')
			print(auxiliary_stat_no_bv)
			print(auxiliary_bf_no_bv)
			if stat_name not in temperature_stats[phy]['BV']:
				temperature_stats[phy]['BV'][stat_name] = np.average(auxiliary_stat_bv)
				temperature_stats[phy]['BV']['BF'] = np.average(auxiliary_bf_bv)
			if stat_name not in temperature_stats[phy]['No_BV']:
				temperature_stats[phy]['No_BV'][stat_name] = np.average(auxiliary_stat_no_bv)
				temperature_stats[phy]['No_BV']['BF'] = np.average(auxiliary_bf_no_bv)
			auxiliary_stat_bv = []
			auxiliary_bf_bv = []
			auxiliary_stat_no_bv = []
			auxiliary_bf_no_bv = []
		self.print_dictionary(temperature_stats)
		xlabel = 'Beating Frequency (Hz)'
		if stat_name_t is not None and stat_name_t == 'PER':
			ylabel = 'PER (%)'
			plot_title = f'PER vs Beating Frequency for {temperature}'
			fig_name = f'plot_PER_vs_BF_{temperature}_{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}.png'
		else:
			ylabel = f'{stat_name} (%)'
			plot_title = f'{stat_name} vs Beating Frequency for {temperature}'
			fig_name = f'plot_{stat_name}_vs_BF_{temperature}_{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}.png'
		fig_save_path = '/home/burhanuddin/Desktop/osf/logparser/Graphs/Increasing_Temperature/All_phys_graphs'+'/'+fig_name
		plotter.plot_prr_v_temperature_phy(temperature_stats, xlabel,ylabel,plot_title, stat_name)
		
		plt.savefig(fig_save_path)
		# plt.show()

	def plot_avg_bf_v_temperature_all_phys(self, heating_node, bv_mode, src, fwd, dst, physical_layer=None):
		"""
			This function plots the Beating Frequency vs temperature for all physical layers.
			:param heating_node: Node which is being heated
			:param bv_mode: Mode of bit voting
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:return: None
		"""

		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab'
		if heating_node == 'fwd':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Forwarder'
		elif heating_node == 'src':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10'

		reader = FileReader(path)
		temperature_bf_stats = {'PHY_BLE_2M':{
			'temperature_bf':{}
		}, 'PHY_BLE_1M':{
			'temperature_bf':{}
		}, 'PHY_BLE_500K':{
			'temperature_bf':{}
		}, 'PHY_BLE_125K':{
			'temperature_bf':{}
		}}

		csv_files_dict = reader.get_dcube_csv_files_pair_temperature(bv_mode, str(src), str(fwd), str(dst))
		# self.print_dictionary(csv_files_dict)
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)

		for phy, phy_stat_temp_csv_files in csv_files_dict.items():
			print(f"--------------- PHY: {phy} -----------------------")
			auxiliary_bf = []
			for temperature, csv_files in phy_stat_temp_csv_files.items():
				print(f"--------------- Temperature: {temperature} -----------------------")
				for csv_file in csv_files:
					print(f"Reading file {csv_file}\n")
					data = pd.read_csv(csv_file)
					if not data.empty:
							auxiliary_bf.append(data['Beating_Frequency'].values[0])
				temperature = int(temperature[:-1])
				if temperature not in temperature_bf_stats[phy]['temperature_bf']:
					temperature_bf_stats[phy]['temperature_bf'][temperature] = np.average(auxiliary_bf)
				else:
					temperature_bf_stats[phy]['temperature_bf'][temperature] = np.average(auxiliary_bf)
				auxiliary_bf = []

		self.print_dictionary(temperature_bf_stats)
		# plotter.plot_bf_v_temperature_all_phy(temperature_bf_stats, bv_mode)
		if physical_layer is not None:
			plotter.plot_bf_v_temperature_all_phy(temperature_bf_stats[physical_layer], bv_mode, physical_layer)
			fig_save_path = '/home/burhanuddin/Desktop/osf/logparser/Graphs/Increasing_Temperature/'+physical_layer+'/'+'BF_vs_Temp/'+f'plot_BF_vs_{bv_mode}_Temp_{physical_layer}_{datetime.datetime.now().strftime("%Y%m%d%H%M%S")}.png'
			plt.savefig(fig_save_path)
		plt.show()
	
	def plot_average_corrections_vs_temperature(self, heating_node, bv_mode, src, fwd, dst, physical_layer=None):
		"""
			This function plots the average corrections vs temperature for all or specific physical layer.
			:param heating_node: Node which is being heated
			:param bv_mode: Mode of bit voting
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:param physical_layer: Physical layer used for the experiment
			:return: None
		"""
		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab'
		if heating_node == 'fwd':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Forwarder'
		elif heating_node == 'src':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10'

		corrections_temperature_stats = {'PHY_BLE_2M':{
			'temperature_corrections':{}
		}, 'PHY_BLE_1M':{
			'temperature_corrections':{}
		}, 'PHY_BLE_500K':{
			'temperature_corrections':{}
		}, 'PHY_BLE_125K':{
			'temperature_corrections':{}
		}}

		reader = FileReader(path)
		csv_files_dict = reader.get_dcube_csv_files_pair_temperature(bv_mode, str(src), str(fwd), str(dst))
		# self.print_dictionary(csv_files_dict)
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)

		for phy, phy_stat_temp_csv_files in csv_files_dict.items():
			print(f"--------------- PHY: {phy} -----------------------")
			auxiliary_corrections = []
			for temperature, csv_files in phy_stat_temp_csv_files.items():
				print(f"--------------- Temperature: {temperature} -----------------------")
				for csv_file in csv_files:
					print(f"Reading file {csv_file}\n")
					data = pd.read_csv(csv_file)
					if not data.empty:
							auxiliary_corrections.append(data['Total_Corrections'].values[0])
				temperature = int(temperature[:-1])
				print('Corrections: ', auxiliary_corrections)
				if temperature not in corrections_temperature_stats[phy]['temperature_corrections']:
					print('average: ', np.average(auxiliary_corrections))
					corrections_temperature_stats[phy]['temperature_corrections'][temperature] = np.average(auxiliary_corrections)

				auxiliary_corrections = []

		self.print_dictionary(corrections_temperature_stats)
		plotter.plot_corrections_v_temperature_all_phy(corrections_temperature_stats, bv_mode)

		plt.show()

	def plot_average_corrections_vs_bf_one_phy(self, heating_node, src, fwd, dst, physical_layer):
		"""
			This function plots the average corrections vs Beating Frequency for a specific physical layer.
			:param heating_node: Node which is being heated
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:param physical_layer: Physical layer used for the experiment
			:return: None
		"""
		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab'
		if heating_node == 'fwd':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Forwarder'
		elif heating_node == 'src':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10'

		reader = FileReader(path)
		csv_files_dict = reader.get_dcube_csv_files_pair_temperature(True, str(src), str(fwd), str(dst))
		# self.print_dictionary(csv_files_dict)
		plotter = DataPlotter(path, physical_layer, 255, False)
		csv_files_tempwise = csv_files_dict[physical_layer]
		auxiliary_corrections = []
		auxiliary_bf = []
		print(f"--------------- PHY: {physical_layer} -----------------------")
		for temperature, csv_files in csv_files_tempwise.items():
			auxiliary_corrections = []
			auxiliary_bf = []
			print(f"--------------- Temperature: {temperature} -----------------------")
			for csv_file in csv_files:
				print(f"Reading file {csv_file}\n")
				data = pd.read_csv(csv_file)
				if not data.empty:
					auxiliary_corrections.append(data['Total_Corrections'].values[0])
					auxiliary_bf.append(data['Beating_Frequency'].values[0])
			temp = int(temperature[:-1])
			print('Corrections: ', auxiliary_corrections)
			print('Beating Frequency: ', auxiliary_bf)
			if  'Corrections' not in self.phy_wise_stats['BV']:
				print('average: ', np.average(auxiliary_corrections))
				self.phy_wise_stats['BV']['Corrections'] = []
				self.phy_wise_stats['BV']['BF'] = []
				self.phy_wise_stats['BV']['Corrections'].append(np.average(auxiliary_corrections))
				self.phy_wise_stats['BV']['BF'].append(np.average(auxiliary_bf))
			else:
				self.phy_wise_stats['BV']['Corrections'].append(np.average(auxiliary_corrections))
				self.phy_wise_stats['BV']['BF'].append(np.average(auxiliary_bf))
			auxiliary_corrections = []
			auxiliary_bf = []

		self.print_dictionary(self.phy_wise_stats)
		plotter.plot_beating_v_per_phy(self.phy_wise_stats, 'BV', 'Beating Frequency (Hz)', 'Average Corrections', 'Average Corrections vs Beating Frequency', 'Corrections', physical_layer, True)

		plt.show()


	def plot_avg_bf_per_across_temp_all_phys(self, heating_node, bv_mode, src, fwd, dst, physical_layer=None):
		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab'
		if heating_node == 'fwd':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Forwarder'
		elif heating_node == 'src':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10'

		corrections_temperature_stats = {'PHY_BLE_2M':{
		}, 'PHY_BLE_1M':{
		}, 'PHY_BLE_500K':{
		}, 'PHY_BLE_125K':{
		}}

		reader = FileReader(path)
		csv_files_dict = reader.get_dcube_csv_files_pair_temperature(bv_mode, str(src), str(fwd), str(dst))		
		self.print_dictionary(csv_files_dict)
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)
		for phy, phy_stat_temp_csv_files in csv_files_dict.items():
			print(f"--------------- PHY: {phy} -----------------------")
			auxiliary_stat = []
			auxiliary_bf = []
			for temperature, csv_files in phy_stat_temp_csv_files.items():
				print(f"--------------- Temperature: {temperature} -----------------------")
				for csv_file in csv_files:
					print(f"Reading file {csv_file}\n")
					data = pd.read_csv(csv_file)
					if not data.empty:
						auxiliary_stat.append((100 - data['PRR'].values[0]))
						auxiliary_bf.append(data['Beating_Frequency'].values[0])
				temperature = int(temperature[:-1])
				print('PER: ', auxiliary_stat)
				if 'PER' not in corrections_temperature_stats[phy]:
					corrections_temperature_stats[phy]['PER'] = []
					corrections_temperature_stats[phy]['BF'] = []
					corrections_temperature_stats[phy]['PER'].append(np.average(auxiliary_stat))
					corrections_temperature_stats[phy]['BF'].append(np.average(auxiliary_bf))
				else:
					corrections_temperature_stats[phy]['PER'].append(np.average(auxiliary_stat))
					corrections_temperature_stats[phy]['BF'].append(np.average(auxiliary_bf))


			auxiliary_stat = []
			auxiliary_bf = []
		self.print_dictionary(corrections_temperature_stats)
		xlabel='Beating Frequency (Hz)'
		ylabel='PER (%)'
		plot_title = f'PER vs Beating Frequency'
		if bv_mode:
			mode = 'BV'
		else:
			mode = 'No_BV'
		plotter.plot_beating_v_per_phy(corrections_temperature_stats, mode,xlabel, ylabel, plot_title)
		plt.show()

	def plot_avg_bf_per_across_temp_one_phy(self, heating_node, src, fwd, dst, physical_layer, stat_name):
		"""
			This function plots the beating frequency vs PER/PRR/PDR for a specific physical layer, with temperature data
			:param heating_node: Node which is being heated
			:param src: Source node id
			:param fwd: Forwarder node id
			:param dst: Destination node id
			:param physical_layer: Physical layer for which the plot is to be drawn
			:param stat_name: Name of the statistic that we want to plot against temperature
			:return: None
		"""
		path = '/home/burhanuddin/Desktop/osf/logparser/Dcube_Logs/Templab'
		if heating_node == 'fwd':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Forwarder'
		elif heating_node == 'src':
			path = path + '/Neg8dBm/Increasing_Temperature/Heating_Source/Heating_Source_inc_10'

		reader = FileReader(path)
		temperature_stats = {
			'BV':{
				'BF':[],
			},
			'No_BV':{
				'BF':[],
			}
		}
		csv_files_dict_bv = reader.get_dcube_csv_files_pair_temperature(True, str(src), str(fwd), str(dst))
		reader_2 = FileReader(path)
		csv_files_dict_no_bv = reader_2.get_dcube_csv_files_pair_temperature(False, str(src), str(fwd), str(dst))
		# self.print_dictionary(csv_files_dict_bv)
		# self.print_dictionary(csv_files_dict_no_bv)
		# sys.exit(1)
		stat_name_t = stat_name
		if stat_name == 'PER':
			stat_name = 'PRR'
			stat_name_t = 'PER'
		plotter = DataPlotter(path, 'PHY_BLE_2M', 64, False)

		#BV
		print('--------------BV------------------')
		print(f"--------------- PHY: {physical_layer} -----------------------")
		# sys.exit(1)
		physical_layer_dict_bv = dict(sorted(csv_files_dict_bv[physical_layer].items()))
		physical_layer_dict_no_bv = dict(sorted(csv_files_dict_no_bv[physical_layer].items()))
		for (temperature_bv, csv_files_bv), (temperature_no_bv, csv_files_no_bv) in zip(physical_layer_dict_bv.items(), physical_layer_dict_no_bv.items()):
			auxiliary_stat_bv = []
			auxiliary_bf_bv = []
			auxiliary_stat_no_bv = []
			auxiliary_bf_no_bv = []
			print(f'--------------- BV: {temperature_bv} No_BV:{temperature_no_bv}-----------------------')
			for (csv_file_bv, csv_file_no_bv) in zip(csv_files_bv, csv_files_no_bv):
				print(f"Reading file {csv_file_bv}\n")
				print(f"Reading file {csv_file_no_bv}\n")
				data_bv = pd.read_csv(csv_file_bv)
				data_no_bv = pd.read_csv(csv_file_no_bv)
				if not data_bv.empty:
						if stat_name_t and stat_name_t == 'PER':
							auxiliary_stat_bv.append(100 - data_bv[stat_name].values[0])
						else:
							auxiliary_stat_bv.append(data_bv[stat_name].values[0])
						auxiliary_bf_bv.append(data_bv['Beating_Frequency'].values[0])
				if not data_no_bv.empty:
						if stat_name_t and stat_name_t == 'PER':
							auxiliary_stat_no_bv.append(100 - data_no_bv[stat_name].values[0])
						else:
							auxiliary_stat_no_bv.append(data_no_bv[stat_name].values[0])
						auxiliary_bf_no_bv.append(data_no_bv['Beating_Frequency'].values[0])
			print('---------BV---------\n')
			print(auxiliary_stat_bv)
			print(auxiliary_bf_bv)
			print('---------NO BV---------\n')
			print(auxiliary_stat_no_bv)
			print(auxiliary_bf_no_bv)
			if stat_name_t not in temperature_stats['BV']:
				temperature_stats['BV'][stat_name_t] = []
				temperature_stats['BV']['BF'] = []
				temperature_stats['BV'][stat_name_t].append(np.average(auxiliary_stat_bv))
				temperature_stats['BV']['BF'] .append(np.average(auxiliary_bf_bv))
			else:
				temperature_stats['BV'][stat_name_t].append(np.average(auxiliary_stat_bv))
				temperature_stats['BV']['BF'] .append(np.average(auxiliary_bf_bv))
			if stat_name_t not in temperature_stats['No_BV']:
				temperature_stats['No_BV'][stat_name_t] = []
				temperature_stats['No_BV']['BF'] = []
				temperature_stats['No_BV'][stat_name_t].append(np.average(auxiliary_stat_no_bv))
				temperature_stats['No_BV']['BF'].append(np.average(auxiliary_bf_no_bv))
			else:
				temperature_stats['No_BV'][stat_name_t].append(np.average(auxiliary_stat_no_bv))
				temperature_stats['No_BV']['BF'] .append(np.average(auxiliary_bf_no_bv))
			auxiliary_stat_bv = []
			auxiliary_bf_bv = []
			auxiliary_stat_no_bv = []
			auxiliary_bf_no_bv = []
		self.print_dictionary(temperature_stats)
		xlabel = 'Beating Frequency (Hz)'
		ylabel = stat_name_t+' (%)'
		plot_title = f'{stat_name_t} vs Beating Frequency'

		plotter.plot_beating_v_per_phy(temperature_stats, True,xlabel,ylabel,plot_title, stat_name_t,physical_layer, True,)
		
		plt.show()

if __name__ == '__main__':
	arg_parser = argparse.ArgumentParser()
	arg_parser.add_argument('-bv', '--bit_voting', type=int, help='Whether Bit Voting error correction scheme is enabled or not', required=True)
	arg_parser.add_argument('-pw', '--pairwise', type=int, help='Whether the plotting should be done pairwise or not', required=True)
	arg_parser.add_argument('-stat', '--stat_name', type=str, help='Name of the statistic that we want to plot', required=True)
	arg_parser.add_argument('-phy', '--physical_layer', type=str, help='Physical layer for which the plot is to be drawn', required=True)
	arg_parser.add_argument('-temp', '--temperature', type=str, help='Temperature for which the plot is to be drawn', required=True)
	args = arg_parser.parse_args()
	print(args.bit_voting)
	stat_plotter = StatsPlotter()

	if args.bit_voting == 1:
		bv = True
	else:
		bv = False
	
	if args.pairwise == 1:
		pair_wise = True
	else:
		pair_wise = False


	#Plot beating vs PRR/PDR for all physical layers
	# stat_plotter.plot_beating_vs_prr_pdr_all_phys('Neg8dBm', bv, 'PDR', pair_wise, 122, 126, 124)
	# stat_plotter.plot_beating_vs_prr_pdr_all_phys('Neg8dBm', bv, 'PDR', pair_wise, 122, 126, 124)
	# plt.show()

	#Plot beating vs PDR/PRR for a specific physical layer
	# stat_plotter.plot_beating_v_prr_pdr_phywise('Neg8dBm', 'PHY_BLE_1M', 'PRR')

	#plotting average corrections vs beating frequency
	# stat_plotter.plot_average_corrections_v_bf('Neg8dBm', True, 122, 126, 124)
    
	#Plot beating vs total corrections for a specific physical layer
	# stat_plotter.plot_beating_freq_vs_corrections('Neg8dBm', 'PHY_BLE_500K')

	#Plot PRR/PDR vs temperature for all physical layers
	# stat_plotter.plot_avg_prr_v_temperature_all_phys('src', bv, 122, 126, 124, 'PRR')
	# stat_plotter.plot_avg_prr_v_temperature_one_phy('src', 122, 126, 124, args.stat_name, args.physical_layer)

	# #Plot Temperature vs Beating Frequency for all physical layers
	# stat_plotter.plot_avg_bf_v_temperature_all_phys('src', bv, 122, 126, 124, args.physical_layer)

	#Plot average corrections vs temperature for all physical layers
	# stat_plotter.plot_average_corrections_vs_temperature('src', bv, 122, 126, 124)
	stat_plotter.plot_average_corrections_vs_bf_one_phy('src', 122, 126, 124, args.physical_layer)

	#Plot PRR/PDR vs temperature for all physical layers for a specific temperature
	# stat_plotter.plot_avg_prr_v_bf_temperature_all_phys('src', 122, 126, 124, args.temperature, args.stat_name)

	#Plot BF vs PER for all physical layer(s)
	# stat_plotter.plot_avg_bf_per_across_temp_all_phys('src',bv, 122, 126, 124, 'PHY_BLE_1M')

	#Plot BF vs PER/PRR/PDR for a specific physical layer with temperature data
	# stat_plotter.plot_avg_bf_per_across_temp_one_phy('src', 122, 126, 124, args.physical_layer, args.stat_name)