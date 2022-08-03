#!/usr/bin/python3
import pandas as pd
import numpy as np
import re

import seaborn as sns
import matplotlib.pyplot as plt  # general plotting
from mpl_toolkits.axes_grid1.inset_locator import inset_axes, mark_inset
import matplotlib.colors as mcolors

import baddplotter as bplot
import baddparser as bparse

import itertools

# Plotting options
# plt.rc('font', family='sans-serif', weight='bold')
# plt.rc('xtick', labelsize=15)
# plt.rc('ytick', labelsize=15)
# plt.rc('axes', labelsize=20, labelweight='bold')
sns.set_context("paper")
plt.rcParams['axes.xmargin'] = 0

sns.set_context("poster")
# sns.set_palette('cividis')
sns.set_palette('colorblind')
# sns.set_palette('viridis')

# Pandas options
pd.set_option('display.max_rows', 100)
pd.set_option('display.min_rows', 50)
pd.set_option('display.max_columns', 20)
pd.set_option('display.width', 1000)
pd.set_option('display.max_colwidth', 50)
pd.set_option('display.float_format', lambda x: '%.3f' % x)

# -------------------------------------------------------------------------
def get_phy(df):
    """Create a PHY column from the description column."""
    search = []
    for values in df['description']:
        search.append(re.search(r'MPHY\d*|_phy[a-zA-Z\d]+', values).group().replace('_phy','').replace('ble','').upper())
    df['phy'] = search
    return df

# -------------------------------------------------------------------------
def get_pwr(df):
    """Create a PWR column from the description column."""
    search = []
    for values in df['description']:
        search.append(re.search(r'_pwr[a-zA-Z\d]+', values).group().replace('_pwr','').capitalize().replace("b", "B"))
    df['pwr'] = search
    return df

# -------------------------------------------------------------------------
def get_rntx(df):
    """Create a RTNX column from the description column."""
    search = []
    for values in df['description']:
        result = re.search(r'RNTX', values)
        search.append(1) if result else search.append(0)
    df['rntx'] = search
    return df

if __name__ == "__main__":

# -----------------------------------------------------------------------------
# Dissemination
# -----------------------------------------------------------------------------
    fig, axes = plt.subplots(3, 3, figsize=(18, 18), sharey=False)
    df = pd.read_csv('/home/michael/EWSN2022/dissemination.csv', index_col=0)

    df = get_phy(df)
    df = get_pwr(df)


    df = df[['jamming', 'phy', 'pwr', 'reliability', 'layout', 'lat_combined', 'energy_total', 'len']]
    df.rename(columns = {'lat_combined':'lat', 'energy_total':'energy'}, inplace = True)

    mymap = {'None': 0}
    df = df.applymap(lambda s: mymap.get(s) if s in mymap else s)
    df['lat'] = df['lat'].astype(float)/1000

    # Reliability
    sns.barplot(ax=axes[0, 0], x='phy', y='reliability', data=df[df['jamming'] == 0], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[0, 1], x='phy', y='reliability', data=df[df['jamming'] == 1], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[0, 2], x='phy', y='reliability', data=df[df['jamming'] == 3], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')

    plt.tight_layout()
    plt.subplots_adjust(wspace=0, hspace=.35)

    axes[0, 0].get_legend().remove()
    axes[0, 1].get_legend().remove()
    # axes[0, 2].get_legend().remove()

    # handles, labels_orig = axes[0, 2].get_legend_handles_labels()
    # axes[0, 2].legend(handles=handles, loc='best', ncol=3, bbox_to_anchor=(1.2,1.3), fontsize=24)
    axes[0, 2].legend(loc='best', fontsize=20)

    axes[0, 0].set_xlabel(None)
    axes[0, 1].set_xlabel(None)
    axes[0, 2].set_xlabel(None)

    axes[0, 0].set_ylim(0,100)
    axes[0, 1].axes.yaxis.set_visible(False)
    axes[0, 2].axes.yaxis.set_visible(False)

    axes[0, 0].set_ylabel('Reliability (%)', labelpad=18)

    axes[0, 0].set_title('No Interf.')
    axes[0, 1].set_title('Mild Interf.')
    axes[0, 2].set_title('Strong Interf.')

    axes[0, 1].axvspan(-0.5, 4.5, facecolor='lightcoral', alpha=0.3, zorder=0)
    axes[0, 2].axvspan(-0.5, 4.5, facecolor='red', alpha=0.3, zorder=0)

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    # Energy
    sns.barplot(ax=axes[1, 0], x='phy', y='energy', data=df[df['jamming'] == 0], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[1, 1], x='phy', y='energy', data=df[df['jamming'] == 1], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[1, 2], x='phy', y='energy', data=df[df['jamming'] == 3], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')

    axes[1, 0].get_legend().remove()
    axes[1, 1].get_legend().remove()
    axes[1, 2].get_legend().remove()

    axes[1, 0].set_xlabel(None)
    axes[1, 1].set_xlabel(None)
    axes[1, 2].set_xlabel(None)

    axes[1, 1].axes.yaxis.set_visible(False)
    axes[1, 2].axes.yaxis.set_visible(False)

    axes[1, 0].set_ylabel('Energy (J)')

    axes[1, 1].axvspan(-0.5, 4.5, facecolor='lightcoral', alpha=0.3, zorder=0)
    axes[1, 2].axvspan(-0.5, 4.5, facecolor='red', alpha=0.3, zorder=0)

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    # Latency
    sns.barplot(ax=axes[2, 0], x='phy', y='lat', data=df[df['jamming'] == 0], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[2, 1], x='phy', y='lat', data=df[df['jamming'] == 1], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[2, 2], x='phy', y='lat', data=df[df['jamming'] == 3], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')

    axes[2, 0].get_legend().remove()
    axes[2, 1].get_legend().remove()
    axes[2, 2].get_legend().remove()

    axes[2, 0].set_xlabel(None)
    axes[2, 1].set_xlabel(None)
    axes[2, 2].set_xlabel(None)

    axes[2, 1].axes.yaxis.set_visible(False)
    axes[2, 2].axes.yaxis.set_visible(False)

    axes[2, 0].set_ylabel('Latency (ms)')

    axes[2, 1].axvspan(-0.5, 4.5, facecolor='lightcoral', alpha=0.3, zorder=0)
    axes[2, 2].axvspan(-0.5, 4.5, facecolor='red', alpha=0.3, zorder=0)

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    fig.savefig('/home/michael/EWSN2022/fig_dissemination.pdf', bbox_inches='tight')

# -----------------------------------------------------------------------------
# Dense RNTX and Backoff
# -----------------------------------------------------------------------------
    df = pd.read_csv('/home/michael/EWSN2022/dense_rntx_backoff.csv', index_col=0)

    df = get_phy(df)
    df = get_pwr(df)

    df = df[['jamming', 'phy', 'pwr', 'reliability', 'layout', 'lat_combined', 'energy_total', 'len', 'type']]
    df.rename(columns = {'lat_combined':'lat', 'energy_total':'energy'}, inplace = True)

    mymap = {'None': 0}
    df = df.applymap(lambda s: mymap.get(s) if s in mymap else s)
    df['lat'] = df['lat'].astype(float)/1000

    fig, axes = plt.subplots(1, 2, figsize=(12, 6), sharey=False)
    sns.barplot(ax=axes[0], x='type', y='reliability', data=df, edgecolor='black')
    sns.barplot(ax=axes[1], x='type', y='energy', data=df, edgecolor='black')
    # sns.barplot(ax=axes[2], x='type', y='lat', data=df, edgecolor='black')

    plt.tight_layout()
    plt.subplots_adjust(wspace=0.35)

    axes[0].set_ylabel('Reliability (%)')
    axes[1].set_ylabel('Energy (J)')
    # axes[2].set_ylabel('Latency (ms)')
    axes[0].set_xlabel(None)
    axes[1].set_xlabel(None)
    # axes[2].set_xlabel(None)

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    fig.savefig('/home/michael/EWSN2022/fig_rntx_backoff.pdf', bbox_inches='tight')


# -----------------------------------------------------------------------------
# Collection
# -----------------------------------------------------------------------------
    fig, axes = plt.subplots(3, 3, figsize=(18, 18), sharey=False)
    df = pd.read_csv('/home/michael/EWSN2022/collection.csv', index_col=0)

    df = get_phy(df)
    df = get_pwr(df)


    df = df[['jamming', 'phy', 'pwr', 'reliability', 'layout', 'lat_combined', 'energy_total', 'len']]
    df.rename(columns = {'lat_combined':'lat', 'energy_total':'energy'}, inplace = True)

    mymap = {'None': 0}
    df = df.applymap(lambda s: mymap.get(s) if s in mymap else s)
    df['lat'] = df['lat'].astype(float)/1000

    # Reliability
    sns.barplot(ax=axes[0, 0], x='phy', y='reliability', data=df[df['jamming'] == 0], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[0, 1], x='phy', y='reliability', data=df[df['jamming'] == 1], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[0, 2], x='phy', y='reliability', data=df[df['jamming'] == 3], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')

    plt.tight_layout()
    plt.subplots_adjust(wspace=0, hspace=.35)

    axes[0, 0].get_legend().remove()
    axes[0, 1].get_legend().remove()
    # axes[0, 2].get_legend().remove()

    # handles, labels_orig = axes[0, 2].get_legend_handles_labels()
    # axes[0, 2].legend(handles=handles, loc='best', ncol=3, bbox_to_anchor=(1.2,1.3), fontsize=24)
    axes[0, 2].legend(loc='best', fontsize=20)

    axes[0, 0].set_xlabel(None)
    axes[0, 1].set_xlabel(None)
    axes[0, 2].set_xlabel(None)

    axes[0, 0].set_ylim(0,100)
    axes[0, 1].axes.yaxis.set_visible(False)
    axes[0, 2].axes.yaxis.set_visible(False)

    axes[0, 0].set_ylabel('Reliability (%)', labelpad=18)

    axes[0, 0].set_title('No Interf.')
    axes[0, 1].set_title('Mild Interf.')
    axes[0, 2].set_title('Strong Interf.')

    axes[0, 1].axvspan(-0.5, 4.5, facecolor='lightcoral', alpha=0.3, zorder=0)
    axes[0, 2].axvspan(-0.5, 4.5, facecolor='red', alpha=0.3, zorder=0)

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    # Energy
    sns.barplot(ax=axes[1, 0], x='phy', y='energy', data=df[df['jamming'] == 0], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[1, 1], x='phy', y='energy', data=df[df['jamming'] == 1], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[1, 2], x='phy', y='energy', data=df[df['jamming'] == 3], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')

    axes[1, 0].get_legend().remove()
    axes[1, 1].get_legend().remove()
    axes[1, 2].get_legend().remove()

    axes[1, 0].set_xlabel(None)
    axes[1, 1].set_xlabel(None)
    axes[1, 2].set_xlabel(None)

    axes[1, 1].axes.yaxis.set_visible(False)
    axes[1, 2].axes.yaxis.set_visible(False)

    axes[1, 0].set_ylabel('Energy (J)')

    axes[1, 1].axvspan(-0.5, 4.5, facecolor='lightcoral', alpha=0.3, zorder=0)
    axes[1, 2].axvspan(-0.5, 4.5, facecolor='red', alpha=0.3, zorder=0)

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    # Latency
    sns.barplot(ax=axes[2, 0], x='phy', y='lat', data=df[df['jamming'] == 0], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[2, 1], x='phy', y='lat', data=df[df['jamming'] == 1], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')
    sns.barplot(ax=axes[2, 2], x='phy', y='lat', data=df[df['jamming'] == 3], hue='len', order=['125K', 'IEEE', '500K', '1M', '2M'], edgecolor='black')

    axes[2, 0].get_legend().remove()
    axes[2, 1].get_legend().remove()
    axes[2, 2].get_legend().remove()

    axes[2, 0].set_xlabel(None)
    axes[2, 1].set_xlabel(None)
    axes[2, 2].set_xlabel(None)

    axes[2, 1].axes.yaxis.set_visible(False)
    axes[2, 2].axes.yaxis.set_visible(False)

    axes[2, 0].set_ylabel('Latency (ms)')

    axes[2, 1].axvspan(-0.5, 4.5, facecolor='lightcoral', alpha=0.3, zorder=0)
    axes[2, 2].axvspan(-0.5, 4.5, facecolor='red', alpha=0.3, zorder=0)

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    fig.savefig('/home/michael/EWSN2022/fig_collection.pdf', bbox_inches='tight')

# -----------------------------------------------------------------------------
# MPHY
# -----------------------------------------------------------------------------
    fig, axes = plt.subplots(3, 3, figsize=(18, 18), sharey=False)
    df = pd.read_csv('/home/michael/EWSN2022/mphy_aperiodic.csv', index_col=0)

    df = get_phy(df)
    df = get_pwr(df)

    df = df[['jamming', 'phy', 'pwr', 'reliability', 'layout', 'lat_combined', 'energy_total', 'len']]
    df.rename(columns = {'lat_combined':'lat', 'energy_total':'energy'}, inplace = True)

    mymap = {'None': 0}
    df = df.applymap(lambda s: mymap.get(s) if s in mymap else s)
    df['lat'] = df['lat'].astype(float)/1000
    df = df[df['layout'] != 5]
    df = df[df['layout'] != 7]

    # Reliability
    sns.barplot(ax=axes[0, 0], x='phy', y='reliability', data=df[df['layout'] == 3], edgecolor='black')
    sns.barplot(ax=axes[0, 1], x='phy', y='reliability', data=df[df['layout'] == 4], edgecolor='black')
    sns.barplot(ax=axes[0, 2], x='phy', y='reliability', data=df[df['layout'] == 6], edgecolor='black')

    plt.tight_layout()
    plt.subplots_adjust(wspace=0, hspace=.35)

    # axes[0, 0].get_legend().remove()
    # axes[0, 1].get_legend().remove()
    # # axes[0, 2].get_legend().remove()

    # handles, labels_orig = axes[0, 2].get_legend_handles_labels()
    # axes[0, 2].legend(handles=handles, loc='best', ncol=3, bbox_to_anchor=(1.2,1.3), fontsize=24)

    axes[0, 0].set_xlabel(None)
    axes[0, 1].set_xlabel(None)
    axes[0, 2].set_xlabel(None)

    axes[0, 0].set_ylim(0,100)
    axes[0, 1].axes.yaxis.set_visible(False)
    axes[0, 2].axes.yaxis.set_visible(False)

    axes[0, 0].set_ylabel('Reliability (%)', labelpad=18)

    axes[0, 0].set_title('All-to-One Layout (47 sources)')
    axes[0, 1].set_title('Sparse Layout (12 sources)')
    axes[0, 2].set_title('Dense Layout (19 sources)')

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    # Energy
    sns.barplot(ax=axes[1, 0], x='phy', y='energy', data=df[df['layout'] == 3], edgecolor='black')
    sns.barplot(ax=axes[1, 1], x='phy', y='energy', data=df[df['layout'] == 4], edgecolor='black')
    sns.barplot(ax=axes[1, 2], x='phy', y='energy', data=df[df['layout'] == 6], edgecolor='black')

    # axes[1, 0].get_legend().remove()
    # axes[1, 1].get_legend().remove()
    # axes[1, 2].get_legend().remove()

    axes[1, 0].set_xlabel(None)
    axes[1, 1].set_xlabel(None)
    axes[1, 2].set_xlabel(None)

    axes[1, 1].axes.yaxis.set_visible(False)
    axes[1, 2].axes.yaxis.set_visible(False)

    axes[1, 0].set_ylabel('Energy (J)')

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    # Latency
    sns.barplot(ax=axes[2, 0], x='phy', y='lat', data=df[df['layout'] == 3], edgecolor='black')
    sns.barplot(ax=axes[2, 1], x='phy', y='lat', data=df[df['layout'] == 4], edgecolor='black')
    sns.barplot(ax=axes[2, 2], x='phy', y='lat', data=df[df['layout'] == 6], edgecolor='black')

    # axes[2, 0].get_legend().remove()
    # axes[2, 1].get_legend().remove()
    # axes[2, 2].get_legend().remove()

    axes[2, 0].set_xlabel(None)
    axes[2, 1].set_xlabel('Fraction of 125K to 2M Utilization (%)')
    axes[2, 2].set_xlabel(None)

    axes[2, 1].axes.yaxis.set_visible(False)
    axes[2, 2].axes.yaxis.set_visible(False)

    axes[2, 0].set_ylabel('Latency (ms)')

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    fig.savefig('/home/michael/EWSN2022/fig_mphy.pdf', bbox_inches='tight')



# -----------------------------------------------------------------------------
# MPHY - Dynamic
# -----------------------------------------------------------------------------
    fig, axes = plt.subplots(2, 3, figsize=(18, 12), sharey=False)
    df = pd.read_csv('/home/michael/EWSN2022/mphy_final.csv', index_col=0)

    df = get_phy(df)
    df = get_pwr(df)

    df = df[['jamming', 'phy', 'pwr', 'reliability', 'layout', 'lat_combined', 'energy_total', 'len', 'periodicity', 'serial']]
    df.rename(columns = {'lat_combined':'lat', 'energy_total':'energy'}, inplace = True)

    mymap = {'None': 0}
    df = df.applymap(lambda s: mymap.get(s) if s in mymap else s)
    df['lat'] = df['lat'].astype(float)/1000
    df = df[df['serial'] == 0]
    df = df[df['periodicity'] == 'Aperiodic']

    # Reliability
    sns.barplot(ax=axes[0, 0], x='phy', y='reliability', data=df[df['layout'] == 3], edgecolor='black')
    sns.barplot(ax=axes[0, 1], x='phy', y='reliability', data=df[df['layout'] == 4], edgecolor='black')
    sns.barplot(ax=axes[0, 2], x='phy', y='reliability', data=df[df['layout'] == 6], edgecolor='black')

    plt.tight_layout()
    plt.subplots_adjust(wspace=0, hspace=.22)

    # axes[0, 0].get_legend().remove()
    # axes[0, 1].get_legend().remove()
    # # axes[0, 2].get_legend().remove()

    # handles, labels_orig = axes[0, 2].get_legend_handles_labels()
    # axes[0, 2].legend(handles=handles, loc='best', ncol=3, bbox_to_anchor=(1.2,1.3), fontsize=24)

    axes[0, 0].set_xlabel(None)
    axes[0, 1].set_xlabel(None)
    axes[0, 2].set_xlabel(None)

    axes[0, 0].set_ylim(0,100)
    axes[0, 1].axes.yaxis.set_visible(False)
    axes[0, 2].axes.yaxis.set_visible(False)

    axes[0, 0].set_ylabel('Reliability (%)', labelpad=18)

    axes[0, 0].set_title('All-to-One Layout (47 sources)')
    axes[0, 1].set_title('Sparse Layout (12 sources)')
    axes[0, 2].set_title('Dense Layout (19 sources)')

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    # Energy
    sns.barplot(ax=axes[1, 0], x='phy', y='energy', data=df[df['layout'] == 3], edgecolor='black')
    sns.barplot(ax=axes[1, 1], x='phy', y='energy', data=df[df['layout'] == 4], edgecolor='black')
    sns.barplot(ax=axes[1, 2], x='phy', y='energy', data=df[df['layout'] == 6], edgecolor='black')

    # axes[1, 0].get_legend().remove()
    # axes[1, 1].get_legend().remove()
    # axes[1, 2].get_legend().remove()

    axes[1, 0].set_xlabel(None)
    axes[1, 1].set_xlabel(None)
    axes[1, 2].set_xlabel(None)

    axes[1, 1].axes.yaxis.set_visible(False)
    axes[1, 2].axes.yaxis.set_visible(False)

    axes[1, 0].set_ylabel('Energy (J)')

    for ax in fig.axes:
        ax.tick_params(axis='x', labelrotation=45)

    # Latency
    # sns.barplot(ax=axes[2, 0], x='phy', y='lat', data=df[df['layout'] == 3], edgecolor='black')
    # sns.barplot(ax=axes[2, 1], x='phy', y='lat', data=df[df['layout'] == 4], edgecolor='black')
    # sns.barplot(ax=axes[2, 2], x='phy', y='lat', data=df[df['layout'] == 6], edgecolor='black')
    #
    # # axes[2, 0].get_legend().remove()
    # # axes[2, 1].get_legend().remove()
    # # axes[2, 2].get_legend().remove()
    #
    # axes[2, 0].set_xlabel(None)
    # axes[2, 1].set_xlabel(None)
    # axes[2, 2].set_xlabel(None)
    #
    # axes[2, 1].axes.yaxis.set_visible(False)
    # axes[2, 2].axes.yaxis.set_visible(False)
    #
    # axes[2, 0].set_ylabel('Latency (ms)')
    #
    # for ax in fig.axes:
    #     ax.tick_params(axis='x', labelrotation=45)

    fig.savefig('/home/michael/EWSN2022/fig_mphy_dynamic.pdf', bbox_inches='tight')
