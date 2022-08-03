#!/usr/bin/python3
"""Scripts for generating batches of templab.csv files."""
#------------------------------------------------------------------------------------------------------------------------------
# Date:         18/11/2020
# Author:       Victor Marot - Toshiba Research Europe Ltd
# Description:  Creates a batch of templab.csv files according to the tests specified in an inputted templab_gen.csv.              
#
# Example:      ./templab_csv_generator.py -out=/home/user/atomic/examples/dcube/templab/templab_generation -t=templab_gen.csv
# 
# templab_gen.csv columns:  time = time interval between temperature changes
#                           min = minimum temperature of the test
#                           max = maximum temperature of the test
#                           interval = temperature increment
#                           src = ID of the source node (do not specify "rpi")
#                           fwd = ID of the forwarder node (do not specify "rpi")
#                           dst = ID of the destination node (do not specify "rpi")
#                           type = type of temperature change pattern (options: nALL, nSRC, nDST, nFWD)             
#------------------------------------------------------------------------------------------------------------------------------

import os
import traceback
import sys
import pandas as pd
import numpy as np
import argparse


if __name__ == "__main__":
    # Cmd line args
    ap = argparse.ArgumentParser(prog='AtomicPy', description='Parse and plot Atomic serial logs')
    ap.add_argument('-out', required=False, default=None, help='Takes input csv address')
    ap.add_argument('-t', required=False, default=None, help='Takes input csv file')
    args = ap.parse_args()

    output_path = args.out

    try:
        if args.t:
            print("The templab generation file is located as follow: " + output_path + '/' + args.t)
            
            # Create the templab dataframe firstline data
            column_names = ['timestamp','rpi120','rpi121','rpi122','rpi123','rpi124','rpi125','rpi126','rpi127']
            
            
            if os.path.exists(output_path + '/' + args.t):
                df = pd.read_csv(output_path + '/' + args.t)
                
                # Loops through lines of the generation specification
                for index, row in df.iterrows():
                    dtp = pd.DataFrame(columns=column_names)
                    time_interval = row['time']
                    min_temp = row['min']
                    max_temp = row['max']
                    temp_interval = row['interval']
                    SRC_ID = row['src']
                    FWD_ID = row['fwd']
                    DST_ID = row['dst']
                    pattern_type = row['type']

                    # Gets amount of steps needed in the test
                    iteration = int((max_temp - min_temp)/temp_interval) #- 1
                    newcol = []

                    # Creates a CSV with specification of time interval and steady "ambient" temperature specification (will later be modified to put the right temperature)
                    for x in range(0,iteration+1):
                        timestamp = time_interval*60000000000
                        add = pd.Series([(timestamp*(x)), "30", "30", "30", "30", "30", "30", "30", "30"], index=['timestamp','rpi120','rpi121','rpi122','rpi123','rpi124','rpi125','rpi126','rpi127'])
                        dtp = dtp.append(add, ignore_index=True)
                        newcol.append(min_temp + (x*temp_interval))

                    # Switch case determines which column to change depending on the node to modify and their ID
                    if pattern_type == "nALL":
                        dtp['rpi%i' % DST_ID] = newcol
                        dtp['rpi%i' % SRC_ID] = newcol
                        dtp['rpi%i' % FWD_ID] = newcol
                    elif pattern_type == "nSRC":
                        dtp['rpi%i' % SRC_ID] = newcol
                    elif pattern_type == "nDST":
                        dtp['rpi%i' % DST_ID] = newcol
                    elif pattern_type == "nFWD":
                        dtp['rpi%i' % FWD_ID] = newcol

                    # Adds final line to the templab.csv. This helps beating.py correctly splitting the data of each temperature specification. It can be used as cooldown line if needed
                    add = pd.Series([(timestamp*(x+1)), "30", "30", "30", "30", "30", "30", "30", "30"], index=['timestamp','rpi120','rpi121','rpi122','rpi123','rpi124','rpi125','rpi126','rpi127'])
                    dtp = dtp.append(add, ignore_index=True)

                    # Saves the templab file
                    file_path = 'templab-i' + "%i" % time_interval + '-t' + "%i" % min_temp + '-' + "%i" % max_temp + '-' + pattern_type + "-SRC=" + "%i" % SRC_ID + "-DST=" + "%i" % DST_ID + "-FWD=" + "%i" % FWD_ID + '.csv'
                    os.makedirs(output_path + '/generated/', exist_ok=True)
                    if os.path.exists(output_path + '/generated/' + file_path):
                        os.remove(output_path + '/generated/' + file_path)
                    dtp.to_csv(output_path + '/generated/' + file_path, index=None, header=True)

                    print("Templab file %i done" % (index+1))

                print("All Templab files have been created")

            else: 
                print("File does not exist")

        else:
            print("No Arguments inputted. Exit without operation")

        sys.exit(0)

    except Exception as e:
        traceback.print_exc()
        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        print(e)
        print(exc_type, fname, exc_tb.tb_lineno)
        sys.exit(0)
   