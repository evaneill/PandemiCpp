import pandas as pd
import numpy as np

import re 

from matplotlib import pyplot as plt
import matplotlib

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

def generate_gametree_figure(exp_fpath):
	'''
	Produce a figure with 3 subfigures showing the game depth, average branching factor, and the relationship between the two in different cases of Airlift and Government Grant being present

	E.g
		(axdepth, axbranch, axcompare) = generate_gametree_figure('results/ByGroupRandomAgentGameExperiment')
		plt.show(axdepth)
	'''

	# Get the data for each agent
	exp_df = pd.read_csv(exp_fpath if exp_fpath.endswith('.csv') else exp_fpath+'.csv')

	# Define histogram quantities
	depth_count,depth_bins = np.histogram(exp_df.loc[:,'Depth'],bins=(exp_df.Depth.max().astype(int) - exp_df.Depth.min().astype(int)+1))

	avgbranch_count,avgbranch_bins = np.histogram(exp_df.loc[:,'AvgBranch'],bins=max(round(exp_df.shape[0]/1000),10))

	## Make a figure with 3 plots side-by-side
	depthfig, branchfig, comparefig = plt.figure(), plt.figure(), plt.figure()
	axdepth, axbranch, axcompare = depthfig.add_subplot(1,1,1), branchfig.add_subplot(1,1,1), comparefig.add_subplot(1,1,1)

	# Do the histogram for each of game depth and branching factor
	axdepth.hist(depth_bins[:-1],depth_bins,weights=depth_count)
	axdepth.set_xlabel('Game Depth',fontsize=14)
	axdepth.set_ylabel('Number of Games',fontsize=14)

	axbranch.hist(avgbranch_bins[:-1],avgbranch_bins,weights=avgbranch_count)
	axbranch.set_xlabel('Average Branching Factor',fontsize=14)
	axbranch.set_ylabel('Number of Games',fontsize=14)

	# Define filters that will qualify groupby() of mean Average branching factor:
	#	(Both of these are true when the Airlift/GovernmentGrant Event card was present at *some point* in the game)
	hadAirlift = exp_df.firstAirliftPresence>=0
	hadGovernmentGrant = exp_df.firstGovernmentGrantPresence>=0

	# pd.Series indexed by two boolean and one numerical indices
	branchmean_bydepth_and_events = exp_df.groupby([hadAirlift,hadGovernmentGrant,exp_df.Depth]).AvgBranch.mean() 

	# Plot each of the four cases: with or without each of Airlift, Government Grant
	axcompare.plot(branchmean_bydepth_and_events[True,True],label = 'Both Airlift And G. G. present')
	axcompare.plot(branchmean_bydepth_and_events[True,False],label = 'Only Airlift Present') 
	axcompare.plot(branchmean_bydepth_and_events[False,True],label = 'Only G. G. Present')
	axcompare.plot(branchmean_bydepth_and_events[False,False],label = 'Neither Airlift nor G.G. present')

	axcompare.legend()

	axcompare.set_xlabel('Depth',fontsize=14)
	axcompare.set_ylabel('Average Branching Factor',fontsize=14)

	## Return the total figure
	return (axdepth, axbranch, axcompare)

def generate_value_trend_df(*args):
	# Returns a long-form dataframe with columns ['Turn','n_Cured','nSims','Determinizations','Heuristic','Value Type','Value']
	# 'Value' is a double value record
	# The "Value Type" values are one of "Selected Reward" or "State Value"
	# 
	# args = any number of dataframes resulting from load_df(...)
	# Only requirement is that each have the "Turn<k>SelectedReward" and "Turn<k>StateEval" columns.
	# 
	# 
	# initialize an empty dataframe
	comparison_df = pd.DataFrame(columns=['GameWon','Agent Action','n_Cured','nSims','Determinizations','Heuristic','Value Type','Value'])
	for df in args:
		# For each of the 0,5,10,...,90 turns captured in the measurement
		for i in range(0,95,5):
			# define a temporary dataframe as a subset of the given one with selected reward as the value
			col = df[['GameWon','n_Cured','nSims','Determinizations','Heuristic','Turn'+str(i)+'SelectedReward']] 
			col.columns = ['GameWon','n_Cured','nSims','Determinizations','Heuristic','Value'] 
			col['Agent Action']=int(i)
			col['Value Type']=str('Selected Reward')

			# Concatenate it with the new empty dataframe
			comparison_df = pd.concat([comparison_df,col],axis=0) 
	
			col = df[['GameWon','n_Cured','nSims','Determinizations','Heuristic','Turn'+str(i)+'StateEval']] 
			col.columns = ['GameWon','n_Cured','nSims','Determinizations','Heuristic','Value'] 
			col['Agent Action']=int(i)
			col['Value Type']=str('State Value')

			# Concatenate it with the new empty dataframe
			comparison_df = pd.concat([comparison_df,col],axis=0) 

	comparison_df.loc[comparison_df['Value']==-1,'Value']=np.nan

	return comparison_df