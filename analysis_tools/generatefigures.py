import pandas as pd
import numpy as np

import re 

from matplotlib import pyplot as plt

def generate_hist_comparison(colname,exp1_fpath,exp2_fpath,nbin=20,cumulative=False):
	'''
	Produce a density histogram of column `colname` values between experiment 1 and experiment 2. Uses fpath to get agent name from headers

	Bins (the number of which is specified by nbin) are shared by both experiments, with the automatically generated bins made for `exp1` being used to count `exp2`

	cumulative = True will make it plot a cumulative histogram

	E.g
		To make a comparison of the "Depth" column in a "single sample Naive UCT agent experiment" versus "By Group Random Agent" experiment:

		make_hist_comparison("Depth","results/SingleSampleNaiveUCTAgentExperiment","results/ByGroupRandomAgentGameExperiment")
	'''

	## Get the agent names
	# 1) Get the header files
	exp1_header = open(exp1_fpath+".header").read()
	exp2_header = open(exp2_fpath+".header").read()
	# 2) get the agent names from the header files
	exp1_name = re.findall("Agent Name: (.*)\n",exp1_header)[0]
	exp2_name = re.findall("Agent Name: (.*)\n",exp2_header)[0]

	# Get the data for each agent
	exp1_df = pd.read_csv(exp1_fpath + ".csv")
	exp2_df = pd.read_csv(exp2_fpath + ".csv")

	# Define histogram quantities
	counts_exp1,exp1_bins = np.histogram(exp1_df.loc[:,colname],bins=nbin)
	counts_exp2,exp2_bins = np.histogram(exp2_df.loc[:,colname],bins=exp1_bins) 

	## Make a figure with one plot inside
	fig = plt.figure()
	ax = fig.add_subplot(1,1,1)

	if not cumulative:
		ax.hist(exp1_bins[:-1],exp1_bins,weights = counts_exp1,alpha=.5,density=True,label=exp1_name)
		ax.hist(exp1_bins[:-1],exp1_bins,weights = counts_exp2,alpha=.5,density=True,label=exp2_name)
		
		ax.set_title("Density histogram comparison of "+colname)
		ax.legend()
	else:
		ax.hist(exp1_bins[:-1],exp1_bins,weights = counts_exp1, histtype='step',cumulative=True,density=True,label=exp1_name)
		ax.hist(exp1_bins[:-1],exp1_bins,weights = counts_exp2, histtype='step',cumulative=True,density=True,label=exp2_name)
		
		ax.set_title("Cumulative Density histogram comparison of "+colname)
		ax.legend(loc = "lower right")

	ax.set_ylabel("Density")
	ax.set_xlabel(colname)

	return ax