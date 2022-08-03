"""Baddeley parsing logs since 2020."""
import os
import traceback
import sys
import re
import ast
# import operator
# import argparse
# import binascii
import pandas as pd
# import numpy as np
# from scipy import stats
# from collections import Counter
# from collections import OrderedDict

delimiter = ';'


# -----------------------------------------------------------------------------
class LOG_PARSER:
    """LOG_PARSER class."""

    def read_byte_arrays(bytearray_string):
        if bytearray_string.startswith('bytearray(') and \
                bytearray_string.endswith(')'):
            return bytearray(ast.literal_eval(bytearray_string[10:-1]))
        return bytearray_string

    def csv_to_df(self, file):
        """Create df from csv."""
        df = pd.read_csv(file.name, delimiter=delimiter)
        df = df.dropna(axis=1, how='all')  # drop any ampty columns
        return df

    # -------------------------------------------------------------------------
    def parse_log(self, file_from, pattern):
        """Parse a log using regex and save in new log."""
        # Let's us know this is the first line and we need to write a header.
        write_header = 1
        # open the files
        with open(file_from, 'r', encoding="ISO-8859-1") as f:
            with open('./tmp.txt', 'w') as t:
                # HACK: ignore last line as final line is usually corrupt in logs
                for l in f.readlines()[:-1]:
                    m = pattern.search(l)
                    if m:
                        g = m.groupdict('')
                        # print(g)
                        if write_header:
                            t.write(delimiter.join(g.keys()))
                            t.write('\n')
                            write_header = 0
                        t.write(delimiter.join(g.values()))
                        t.write('\n')
                    continue
        # Remember to close the logs!
        f.close()
        t.close()

        return t

    # -------------------------------------------------------------------------
    def parse_logs(self, dir, log, filters, regex, node_regex):
        """Parse logs according to the regex."""
        df_list = []
        data_re = re.compile(regex)
        node_re = re.compile(node_regex)
        print(' > Searching root directory: \"' + dir + '\"')
        try:
            i = 0
            all_df = None
            for root, dirs, files in sorted(os.walk(dir)):
                head, tail = os.path.split(root)
                path = head + '/'
                # while head:
                #     head, _tail = os.path.split(head)
                # print("h2 " + pref + tail)
                path += tail + '/'
                for filter in filters:
                    if(re.findall(filter, path)):
                        print(" - " + path)
                        found_files = 0
                        for f in sorted(files):
                            found_files = 1
                            print(' ... Scanning \"' + f + '\"')
                            f = path + f
                            i = i + 1
                            fi = open(f, 'rb')
                            datafi = fi.read()
                            fi.close()
                            fo = open(f, 'wb')
                            fo.write(datafi.replace('\x00'.encode(), ''.encode()))
                            fo.close()
                            # check the dir exists, and there is a log there
                            open(f, 'rb')
                            # get node id either from filename or from the log itself
                            id = f.rsplit('_', 1)[1].strip('.txt')
                            if id is None:
                                print("WARN: Could not get node id from filename")
                                tmp = self.parse_log(f, node_re)
                                if (os.path.getsize(tmp.name) != 0):
                                    node_df = self.csv_to_df(tmp)  # convert from csv to df
                                    id = node_df['node'].iloc[-1]  # take last line
                                else:
                                    print('WARN: No regex for node id! Skipping...')
                            # get description from directory name
                            # get data csv
                            tmp = self.parse_log(f, data_re)
                            if (os.path.getsize(tmp.name) != 0):
                                data_df = self.csv_to_df(tmp)  # convert from csv to df
                                data_df['id'] = id
                                print(f)
                                data_df['job'] = f.rsplit('/', 2)[1]  # .strip('.txt')
                                # data_df = format_data(data_df)  # format the df
                                if data_df is not None:
                                    if(log is None):
                                        df_list.append(data_df)
                                    else:
                                        return data_df
                                else:
                                    raise Exception('ERROR: Dataframe was None!')
                            # else:
                                # print('WARN: No regex data!')
                        if found_files and df_list:
                            all_df = pd.concat(df_list, sort=True)
            if all_df is not None:
                all_df = all_df.astype({"id": int})
                print(' > Finished parsing ' + str(i) + ' files')
                return all_df
            else:
                raise Exception('ERROR: Folder does not exist! - ' + dir)
        except Exception as e:
            traceback.print_exc()
            exc_type, exc_obj, exc_tb = sys.exc_info()
            fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
            print(e)
            print(exc_type, fname, exc_tb.tb_lineno)
            sys.exit(0)

# -----------------------------------------------------------------------------
    def __init__(self):
        """Parse logs."""
