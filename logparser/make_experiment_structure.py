import os
import datetime

class ExperimentStructure:

    def __init__(self) -> None:

        self.directory_path = os.path.join(os.getcwd(), 'logs')
        self.today_dir = ''
        self.physical_layers = ['PHY_BLE_2M', 'PHY_BLE_1M', 'PHY_BLE_125K', 'PHY_BLE_500K','PHY_IEEE']
        self.experiment_structure = {'PHY_BLE_2M':'', 'PHY_BLE_1M':'', 'PHY_BLE_125K':'', 'PHY_BLE_500K':'','PHY_IEEE':''}

    def make_main_directory(self):
        """
        This function creates the main directory (logs)for the experiment
        :return: None
        """

        if not os.path.exists(self.directory_path):
            print('Creating main directory: ', self.directory_path)
            os.makedirs(self.directory_path)
    
    def make_today_directory(self):
        """
        This function creates the directory of today's date which will contain the logs, graphs and csv files of the experiment
        :return: None
        """

        today = datetime.datetime.now().strftime("%Y%m%d")
        today_dir = os.path.join(self.directory_path, today)
        if not os.path.exists(today_dir):
            print('Creating directory: ', today_dir)
            os.makedirs(today_dir)
        self.today_dir = today_dir

    def make_physical_layer_directories(self):
        """
        This function creates the directories for each physical layer used in the experiment
        :return: None
        """

        for phy in self.physical_layers:
            phy_dir = os.path.join(self.today_dir, phy)
            if not os.path.exists(phy_dir):
                print('Creating directory: ', phy_dir)
                os.makedirs(phy_dir)
            self.experiment_structure[phy] = phy_dir
    
    def make_physical_layer_subdirs(self, pl_name):
        """
        This function creates the subdirectories (Graphs, CSVFiles) for each physical layer used in the experiment
        :param pl_name: Name of the physical layer
        :return: None
        """

        if pl_name not in self.physical_layers:
            print('Physical layer not found')
            return
        phy_dir = os.path.join(self.today_dir, pl_name)
        sub_dirs = ['Graphs']

        for sub_dir in sub_dirs:
            sub_dir_path = os.path.join(phy_dir, sub_dir)
            if not os.path.exists(sub_dir_path):
                print('Creating directory: ', sub_dir_path)
                os.makedirs(sub_dir_path)
    
    def make_experiment_structure(self):
        """
        This function creates the directory structure for the experiment
        :return: None
        """

        self.make_main_directory()
        self.make_today_directory()
        self.make_physical_layer_directories()
        for phy in self.physical_layers:
            self.make_physical_layer_subdirs(phy)
    
    def get_experiment_structure(self):
        """
        This function returns the directory structure for the experiment
        :return: dictionary of directories
        """

        return self.experiment_structure
    
    def get_physical_layer_directory(self, pl_name):
        """
        This function returns the directory path of the physical layer
        :param pl_name: Name of the physical layer
        :return: directory path
        """

        return self.experiment_structure[pl_name]