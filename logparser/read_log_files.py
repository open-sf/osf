import os

class FileReader:
    """
    This class reads the files in the directory and stores the file paths in a dictionary
    """
    def __init__(self, dp) -> None:
        self.directory_path = dp
        self.filedirectory = []
    
    def get_log_file_directories(self):
        """
        This function gets the directories in the Logoutput folder and stores them in a dictionary
        :return: None
        """
        # Specify the directory path
        sub_dirs = sorted(os.listdir(self.directory_path))
        # Iterate over the subdirectories
        for name in sub_dirs:
            if ('Graphs' != name) and ('CSVFiles' != name):
                if os.path.isdir(os.path.join(self.directory_path, name)):
                    print('------------Adding Directory: ', os.path.join(self.directory_path, name), ' ------------')
                    self.filedirectory[os.path.join(self.directory_path, name)] = []
    
    
    def get_log_file_paths(self):
        """
        This function reads the log files in the directory and stores the file paths in a dictionary
        :return: None
        """
        # self.get_log_file_directories()
        # for dir in self.filedirectory.keys():
        file_names = os.listdir(self.directory_path)
        # Sort the file names based on modification time
        sorted_file_names = sorted(file_names, key=lambda x: os.path.getmtime(os.path.join(self.directory_path+'/'+x)))
        # Read the files in the sorted order
        for filename in sorted_file_names:
            # Construct the full file path
            file_path = os.path.join(self.directory_path+'/'+filename)
            print('------------Adding file: ', file_path, ' ------------')
            # Check if the path is a file
            if os.path.isfile(file_path):
                self.filedirectory.append(file_path)
        return self.filedirectory
    
    def get_csv_file_paths(self):
        """
        This function reads the CSV files in the directory and stores the file paths in a dictionary
        :return: None
        """
        # self.get_log_file_directories()
        # for dir in self.filedirectory.keys():
        if os.path.isdir(self.directory_path+'/CSVFiles'):
            file_names = os.listdir(self.directory_path+'/CSVFiles')

            # Sort the file names based on modification time
            sorted_file_names = sorted(file_names, key=lambda x: os.path.getmtime(os.path.join(self.directory_path+'/CSVFiles/', x)))
            # Read the files in the sorted order
            for filename in sorted_file_names:
                # Construct the full file path
                file_path = os.path.join(self.directory_path+'/CSVFiles/', filename)
                print('------------Adding file: ', file_path, ' ------------')
                # Check if the path is a file
                if os.path.isfile(file_path):
                    self.filedirectory.append(file_path)

        return self.filedirectory
