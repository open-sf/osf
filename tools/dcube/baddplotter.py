#!/usr/bin/python
"""Baddeley plotting things since 2020."""
from __future__ import division

import os       # for makedir
import math
import itertools

import pickle
import matplotlib.pyplot as plt  # general plotting
import seaborn as sns

# Matplotlib settings for graphs (need texlive-full, ghostscript and dvipng)
# plt.rc('font', family='sans-serif', weight='bold')
# plt.rc('xtick', labelsize=18)
# plt.rc('ytick', labelsize=60)
# plt.rc('axes', labelsize=60, labelweight='bold')
# plt.rc('axes', labelweight='bold')
# sns.set_context("poster", font_scale=2.5)
# plt.rcParams['axes.xmargin'] = 0

sns.set_context("notebook")
sns.set_palette('Paired')

marker = itertools.cycle((',', '.', 'o', 'v', '^', '<', '>', '8', 's', 'p', '*', 'h', 'H', 'D', 'd', 'P', 'X'))
marker_list = [',', '.', 'o', 'v', '^', '<', '>', '8', 's', 'p', '*', 'h', 'H', 'D', 'd', 'P', 'X']  # for seaborn
hatch = itertools.cycle(('/', 'x', '\\', '-', '//'))
hatch_list = ['/', 'x', '\\', '//', '-']  # for seaborn


# ----------------------------------------------------------------------------
def is_string(obj):
    """Check if an object is a string."""
    return all(isinstance(elem, str) for elem in obj)


# ----------------------------------------------------------------------------
def add_hatches_bar(ax):
    """Add hatches to seaborn barplot."""
    # Loop over the bars. Seaborn seems to save the patches in order of type rather
    # than where they bars are on the graph, so we use a hacky cast to set the
    # hash for the first x, then the second x, etc.
    bin_len = len(ax.patches)/len(ax.patches)
    for i, bar in enumerate(ax.patches):
        h = int(i / bin_len)
        bar.set_hatch(hatch_list[i])


# ----------------------------------------------------------------------------
def add_hatches_box(ax):
    """Add hatches to seaborn boxplot."""
    for hatch, patch in zip(hatch_list, ax.artists):
        patch.set_hatch(hatch)


# ----------------------------------------------------------------------------
# Results compare
# ----------------------------------------------------------------------------
def autolabel(ax, rects, max_h):
    """Attach a text label above each bar displaying its height."""
    for rect in rects:
        rect_height = math.ceil(rect.get_height())
        # rect_height = rect.get_height()
        if rect_height > max_h:
            text_height = rect_height if rect_height <= max_h else max_h
            ax.text(rect.get_x() + rect.get_width()/2., 1*text_height,
                    '%d' % int(rect_height),
                    ha='center', va='bottom')

# ----------------------------------------------------------------------------
def bar(data, x, y, **kwargs):
    """Plot an seaborn barplot and save."""

    order = kwargs['order'] if 'order' in kwargs else None
    hue = kwargs['hue'] if 'hue' in kwargs else None

    ax = sns.barplot(x=x, y=y, hue=hue, data=data, order=order, edgecolor='black')
    # Loop over the bars. Seaborn seems to save the patches in order of type rather
    # than where they bars are on the graph, so we use a hacky cast to set the
    # hash for the first x, then the second x, etc.
    num_bins = len(data[y].unique())
    bin_len = len(ax.patches)/num_bins
    for i, bar in enumerate(ax.patches):
        h = int(i / bin_len)
        bar.set_hatch(hatch_list[h])

    return ax

# ----------------------------------------------------------------------------
def line(data, **kwargs):
    """Plot an seaborn lineplot and save."""

    # ax = sns.lineplot(data=data, lw=2, dashes=False, markers=marker_list[0:data.shape[1]], markersize=7)
    ax = sns.lineplot(data=data, lw=2, dashes=False, markersize=7)

    return ax

# ----------------------------------------------------------------------------
def save_data(data, groupname, desc, dir):
    # replace spaces with underscores
    desc = desc.replace(' ', '_')
    # check the directory exists
    os.makedirs(dir, exist_ok=True)
    # create a /data folder
    os.makedirs(dir + '/data', exist_ok=True)
    # groupname allows us to append data to an existing file
    if groupname:
        filepath = dir + '/data/dict_' + groupname + '.pkl'
        if os.path.isfile(filepath):
            data_dict = pickle.load(open(filepath, 'rb'))
            data_dict[desc] = data
        else:
            data_dict = {desc:data}
        pickle.dump(data_dict, open(filepath, 'wb'))
        print('   ... Saving data: ' + str(filepath))
    # else we just want to save the data
    else:
        pickle.dump(data, open(dir + '/data/dat_' + desc + '.pkl', 'wb'))
        print('   ... Saving data: ' + dir + '/data/dat_' + desc + '.pkl')

    return data_dict


# ----------------------------------------------------------------------------
def save_fig(fig, ax, df, filename, out, **kwargs):
    """Set figure properties and save as pdf."""
    # get kwargs
    xlabel = kwargs['xlabel'] if 'xlabel' in kwargs else None
    ylabel = kwargs['ylabel'] if 'ylabel' in kwargs else None
    title = kwargs['title'] if 'title' in kwargs else None
    labels = kwargs['labels'] if 'labels' in kwargs else None
    loc = kwargs['loc'] if 'loc' in kwargs else 'best'
    tight = kwargs['tight'] if 'tight' in kwargs else 'tight'
    pltshow = kwargs['pltshow'] if 'pltshow' in kwargs else None

    os.makedirs(out, exist_ok=True)

    set_labels(fig, ax, xlabel, ylabel)
    set_title(fig, ax, title)
    set_legend(fig, ax, labels, loc)

    if not out.endswith('/'):
        out = out + '/'  # make sure that ou ends with a '/'


    print('   ... Saving figure: ' + out + 'fig_' + filename + '.pdf')
    fig.savefig(out + 'fig_' + filename + '.pdf', bbox_inches=tight)

    # close all open figs
    plt.close('all')

    return fig, ax


# ----------------------------------------------------------------------------
def set_labels(fig, ax, xlabel, ylabel):
    if xlabel is not None:
        ax.set_xlabel(xlabel)
    if ylabel is not None:
        ax.set_ylabel(ylabel)


# ----------------------------------------------------------------------------
def set_title(fig, ax, title):
    if title is not None:
        ax.set_title(title, fontweight='bold')

# ----------------------------------------------------------------------------
def set_legend(fig, ax, labels, loc):
    L = ax.get_legend()
    # Manually create the legend from the patches and x axes labels
    if L is None:
        xticklabels = []
        for l in ax.get_xticklabels():
            xticklabels.append(l.get_text())
        ax.legend(handles=ax.patches, labels=xticklabels, loc=loc)
    # We already have a legend...
    else:
        # If we don't want a legend, then remove it
        if loc is None:
            L.remove()
        # Manually set bounding box
        elif loc == 'bbox':
            handles, labels_orig = ax.get_legend_handles_labels()
            ax.legend(handles=handles, loc=loc, bbox_to_anchor=(1.05,1.37), fontsize=8)
        # If we DO want a legend and we don't want to set a manual location...
        else:
            ax.legend(labels=labels, loc=loc)
    # We want to add custom labels to the legend so  replace the text
    if labels is not None:
        L = ax.get_legend()
        i = 0
        for l in labels:
            L.get_texts()[i].set_text(l)
            i = i + 1
