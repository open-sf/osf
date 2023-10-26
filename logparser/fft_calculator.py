from scipy import fftpack
import pandas as pd
from matplotlib import pyplot as plt
import numpy as np
import datetime
from scipy import stats as ss
from scipy import signal as ssig
import os

class FFTCalculator:
    def __init__ (self, main_dp, phy, bv) -> None:
        self.main_dp = main_dp
        self.phy = phy
        self.error_positions = {}
        self.bit_err_freq = []
        self.is_bv = bv

    def get_sorted_error_frequencies(self, error_positions) -> None:
        """
        This function sorts the error positions in ascending order
        :return: None
        """

        if len(error_positions) > 0:
            sorted_dict = dict(sorted(error_positions.items()))
            self.bit_err_freq = [int(i)for i in sorted_dict.values()]
        else:
            print('No_error_found')

    def write_to_file(self, beating_frequency, temperature=None) -> None:
        """
        This function writes the beating frequency to a file
        :param beating_frequency: beating frequency
        :return: None
        """
        if temperature:
            if self.is_bv:
                file_path = self.main_dp + f'/statistics_{temperature}_{self.phy}_bv.csv'
            else:
                file_path = self.main_dp + f'/statistics_{temperature}_{self.phy}_no_bv.csv'
        else:
            if self.is_bv:
                file_path = self.main_dp + f'/statistics_{self.phy}_bv.csv'
            else:
                file_path = self.main_dp + f'/statistics_{self.phy}_no_bv.csv'

        print('---------Writing beating frequency to file: ', file_path,'---------------------')

        if os.path.exists(file_path):
            df = pd.read_csv(file_path)
            df['Beating_Frequency'] = beating_frequency
            df.to_csv(file_path, index=False)
        else:
            df = pd.DataFrame({'Beating_Frequency': [beating_frequency]})
            # df['Beating_Frequency'] = beating_frequency
            print(df)
            df.to_csv(file_path, index=False)



    def fft_calculator(self, err_freq, temperature=None) -> None:
        """
        This function calculates the FFT of the error positions
        :return: None
        """

        self.get_sorted_error_frequencies(err_freq)
        sample_freq = []
        power = []
        sampling_rates = {'PHY_BLE_2M': 2000000, 'PHY_BLE_1M': 1000000, 'PHY_BLE_500K': 500000, 'PHY_BLE_125K': 125000}
        cutoff_freqs = {'PHY_BLE_2M': 200000, 'PHY_BLE_1M': 100000, 'PHY_BLE_500K': 50000, 'PHY_BLE_125K': 12500}
        
        # Sample parameters
        sampling_rate = sampling_rates[self.phy] # Hz
        print('sampling_rate: ',sampling_rate)

        #cutoff frequency selection
        cutoff_freq = cutoff_freqs[self.phy]
        print('cutoff_freq: ',cutoff_freq)

        # Perform FFT
        if len(self.bit_err_freq) > 0:
            
            
            err_cnt_mean = sum(self.bit_err_freq) / len(self.bit_err_freq)
            self.bit_err_freq = [i - err_cnt_mean for i in self.bit_err_freq]

            sos = ssig.cheby2(10, 100, cutoff_freq, 'lp', fs=sampling_rate, output='sos')
            self.bit_err_freq = ssig.sosfilt(sos, self.bit_err_freq)
            
            err_fft = fftpack.fft(self.bit_err_freq)
            
            power = np.abs(err_fft)
            sample_freq = fftpack.fftfreq(len(self.bit_err_freq), d=(1/sampling_rate)) 
            # fft_result = np.fft.fft(self.bit_err_freq)
            # frequencies = np.fft.fftfreq(len(self.bit_err_freq), d=1/sampling_rate)

            # Find index of maximum amplitude (excluding DC component)
            # max_amp_index = np.argmax(np.abs(fft_result[1:])) + 1
            # beating_frequency = np.abs(frequencies[max_amp_index])

            all_peaks, _ = ssig.find_peaks(power)
            peaks_df = pd.DataFrame({'Power': (power[x] for x in all_peaks), 'Freq': (sample_freq[x] for x in all_peaks)})
            peaks_df = peaks_df.sort_values(by=['Power'], ascending=False)
            # print(peaks_df)
            peaks_df = peaks_df[peaks_df.Freq >= 0]
            peak_freq = peaks_df['Freq'].head(6)
            # print(peak_freq)
            if peak_freq.any():
                beating_frequency = int(peak_freq.iloc[0])
                print("Beating Frequency: ", beating_frequency)
            else:
                beating_frequency = 0
                print("Beating Frequency: ", beating_frequency)

            self.write_to_file(beating_frequency, temperature)

        return sample_freq, power, cutoff_freq


# if __name__ == "__main__":
#     tt = get_error_frequencies()
#     fft_calculator(tt)