#!/usr/bin/python3

"""D-Cube Plotting."""

# Example:
#
# ./dcube.py --suite="rpludp" --x="SF" --y="reliability" --start=1 --end=5 --title="test" --out=home/mike/test
import pandas as pd
import baddplotter as bplot
import matplotlib.pyplot as plt  # general plotting
import seaborn as sns
import numpy as np
import argparse
from ast import literal_eval

# Pandas options
pd.set_option('display.max_rows', 100)
pd.set_option('display.min_rows', 50)
pd.set_option('display.max_columns', 20)
pd.set_option('display.width', 1000)
pd.set_option('display.max_colwidth', 50)
pd.set_option('display.float_format', lambda x: '%.3f' % x)

# Plotting options
plt.rc('font', family='sans-serif', weight='bold')
plt.rc('xtick', labelsize=15)
plt.rc('ytick', labelsize=15)
plt.rc('axes', labelsize=18, labelweight='bold')
sns.set_context("paper")
plt.rcParams['axes.xmargin'] = 0

sns.set_context("notebook")
sns.set_palette('colorblind')


# --------------------------------------------------------------------------- #
def get_csv(file):
    """Create df from csv."""
    df = pd.read_csv(file.name)
    df = df.dropna(axis=1, how='all')  # drop any empty columns
    return df


# --------------------------------------------------------------------------- #
def get_columns(df, columns):
    """Get coluns."""
    df = df.fillna(0)
    df = df[columns]
    if 'lat_mean' in columns:
        df['lat_mean'] = df['lat_mean'].astype(int)/1000

    # print(df)
    return df


# --------------------------------------------------------------------------- #
def average_data(df):
    """Average all rows with the same description."""
    ret = pd.DataFrame(columns=('start_id', 'end_id', 'jamming', 'reliability',
                                'lat_mean', 'energy_total', 'description',
                                'received', 'missed', 'superfluous'))
    grouped = df.groupby('description')
    for k,v in grouped:
        row = pd.DataFrame({'start_id': v.id.iat[0],
                            'end_id': v.id.iat[-1],
                            'jamming': int(v.jamming.mean()),
                            'reliability': v.reliability.mean(),
                            'lat_mean': v.lat_mean.mean(),
                            'energy_total': v.energy_total.mean(),
                            'description': k,
                            'received': int(round(v.received.mean())),
                            'missed': int(round(v.missed.mean())),
                            'superfluous': int(round(v.superfluous.mean()))}, index=[0])
        ret = ret.append(row, ignore_index=True)
    ret.set_index(['start_id'], inplace=True, drop=False)
    ret.sort_index(inplace=True)
    return ret


# --------------------------------------------------------------------------- #
# Main
# --------------------------------------------------------------------------- #
if __name__ == "__main__":

    # Cmd line args
    ap = argparse.ArgumentParser(prog='d-cubePy', description='D-CUBE API Parser')
    ap.add_argument('--csv', required=False, default='./dcube_results.csv',  help='Results csv file')
    ap.add_argument('--filename', required=False, default='figure', help='Name of the figure')
    ap.add_argument('--out', required=False, default='.', help='Figure output folder')
    ap.add_argument('--x', required=True, help='')
    ap.add_argument('--y', required=True, help='')
    ap.add_argument('--start', required=False, type=int, help='Integer to start x at (in description, prefaced by a \'_\')')
    ap.add_argument('--end', required=False, type=int, help='Integer to end x at (in description, prefaced by a \'_\')')
    ap.add_argument('--step', required=False, type=int, default=1, help='')
    # ap.add_argument('--compare', required=False, default=False, help='')
    # ap.add_argument('--allsuites', required=False, type=str, default='', help='')
    # ap.add_argument('--allplots', required=False, type=str, default='', help='')
    ap.add_argument('--title', required=True, type=str, default='D-Cube Plot', help='')
    ap.add_argument('--xlabel', required=False, type=str, default=None, help='Y axes label')
    ap.add_argument('--ylabel', required=False, type=str, default=None, help='X axes label')
    ap.add_argument('--loc', required=False, type=str, default='best', help='Legend location')
    ap.add_argument('--labels', required=False, type=str, default=None, help='Legend labels')
    args = ap.parse_args()

    # if args.compare:
    #     print('**** Compare [' + str(args.allplots) + '] in ' + args.out + ' for suites [' + str(args.allsuites) + ']')
    #     bplot.compare(
    #         args.out,
    #         re.findall(r"[\w']+", args.allsuites),
    #         re.findall(r"[\w']+", args.allplots),
    #         None, legend='lower right')
    #     exit(1)

    # Pull csv into pandas df
    df = get_csv(open(args.csv, 'r'))
    df = get_columns(df, ['id', 'name', 'description', 'reliability', 'lat_mean', 'energy_total', 'jamming', 'received', 'missed', 'superfluous'])
    print(df)

    # Average all rows with same description
    # df = average_data(df)

    # if args.x not in df:
    #     # The x metric is not a column or the job name, get it from the description
    #     print(df.description)
    #     df[args.x] = df['description'].str.split(args.x + "_", expand=True)[1]
    #     df[args.x] = df[args.x].str.split("_", expand=True)[0]

    # if args.hue:
    #     args.hue = str(args.hue).split(',')
    #     # Search in name for hue categories
    #     df['type'] = df.apply(lambda x: get_hue(args.hue, x['name']), axis=1)
    # df = df[[args.x, args.y, 'type']]

    fig, ax = plt.subplots()
    ax = bplot.bar(x=args.x, y=args.y, data=df, edgecolor='black')

    args.xlabel = str(args.x).capitalize() if args.xlabel is None else args.xlabel
    args.ylabel = str(args.y).capitalize() if args.ylabel is None else args.ylabel
    args.labels = list(args.labels.split(",")) if args.labels is not None else args.labels
    bplot.save_fig(
        fig, ax, df,
        args.filename, args.out,
        xlabel=args.xlabel,
        ylabel=args.ylabel,
        title=args.title,
        loc=args.loc,
        labels=args.labels)
