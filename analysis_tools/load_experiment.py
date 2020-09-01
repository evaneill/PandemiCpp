import pandas as pd
import numpy as np

import os

def featurizeEventUsage(df):
    # Instantiate new column for how long airlift was present before use, if used (-1 if either never present or present & not used)
    df['AirliftUseTime']=df.AirliftUse.copy()
    df.AirliftUseTime[df.AirliftUseTime>=0] = df.AirliftUseTime[df.AirliftUseTime>=0] - df.firstAirliftPresence[df.AirliftUseTime>=0]
    df.AirliftUseTime[df.AirliftUseTime<0] = np.nan

    # Instantiate new column for how long Government Grant was present before use, if used (-1 if either never present or present & not used)
    df['GovernmentGrantUseTime']=df.GovernmentGrantUse.copy()
    df.GovernmentGrantUseTime[df.GovernmentGrantUseTime>=0] = df.GovernmentGrantUseTime[df.GovernmentGrantUseTime>=0] - df.firstGovernmentGrantPresence[df.GovernmentGrantUseTime>=0]
    df.GovernmentGrantUseTime[df.GovernmentGrantUseTime<0] = np.nan

    # Instantiate new column for how long Quiet Night was present before use, if used (-1 if either never present or present & not used)
    df['QuietNightUseTime']=df.QuietNightUse.copy()
    df.QuietNightUseTime[df.QuietNightUseTime>=0] = df.QuietNightUseTime[df.QuietNightUseTime>=0] - df.firstQuietNightPresence[df.QuietNightUseTime>=0]
    df.QuietNightUseTime[df.QuietNightUseTime<0] = np.nan

    # reset to np.nan any values that were -1 (indicating never seen and/or never used)
    df.loc[df.firstAirliftPresence==-1,'firstAirliftPresence']=np.nan
    df.loc[df.firstGovernmentGrantPresence==-1,'firstGovernmentGrantPresence']=np.nan
    df.loc[df.firstQuietNightPresence==-1,'firstQuietNightPresence']=np.nan

    df.loc[df.AirliftUse==-1,'AirliftUse']=np.nan
    df.loc[df.GovernmentGrantUse==-1,'GovernmentGrantUse']=np.nan
    df.loc[df.QuietNightUse==-1,'QuietNightUse']=np.nan
    
def featurizeCure(df):
    df['n_Cured']=1*(df.BlueCured>0) + 1*(df.YellowCured>0) + 1*(df.BlackCured>0) + 1*(df.RedCured>0)
    # have to account for the fact that measurements can only be made *before* actions are taken, so in any games that are won, have to add the winning cure disease
    df.loc[df.GameWon>0,'n_Cured']+=1

    # In any won game, also set the winning cure action to "Depth" - it was at this point that the cure was made and game was won
    df.loc[(df.GameWon>0) & (df.BlueCured<0),'BlueCured']=df.loc[(df.GameWon>0) & (df.BlueCured<0),'Depth']
    df.loc[(df.GameWon>0) & (df.YellowCured<0),'YellowCured']=df.loc[(df.GameWon>0) & (df.YellowCured<0),'Depth']
    df.loc[(df.GameWon>0) & (df.BlackCured<0),'BlackCured']=df.loc[(df.GameWon>0) & (df.BlackCured<0),'Depth']
    df.loc[(df.GameWon>0) & (df.RedCured<0),'RedCured']=df.loc[(df.GameWon>0) & (df.RedCured<0),'Depth']

def featurizeTrade(df):
    df['Trade_count']=df.Give_count + df.Take_count

def featurizePlayerTurns(df):
    # This is based on their being 3 players
    # ~ Number of end-of-player-turn card draws executed 
    df['PlayerTurns']=round((df.Depth - df.Airlift_count - df.GovernmentGrant_count - df.QuietNight_count)/4 +.25)

def featurize(df):
    featurizeEventUsage(df)
    featurizeCure(df)
    featurizeTrade(df)
    featurizePlayerTurns(df)

def series_compare(df1,df2):
    if isinstance(df1,str) and isinstance(df2,str):
        df1 = pd.read_csv(df1 if df1.endswith('.csv') else df1+'.csv')
        df2 = pd.read_csv(df2 if df2.endswith('.csv') else df2+'.csv')

    comparison_series = pd.concat([df1.min(axis=0),df1.mean(axis=0),df1.max(axis=0),df2.min(axis=0),df2.mean(axis=0),df2.max(axis=0)],axis=1) 
    comparison_series.columns = ['df1 Min','df1 Avg','df1 Max','df2 Min','df2 Avg','df2 Max']

    return comparison_series

def load_df(fpath,K,sims,heuristic,selection="Greedy Expectimax"):
    # fpath (str)   = filepath to experiment (e.g. "results/<>.csv")
    # K (int)       = number of determinizations (e.g. 3)
    # sims (str)    = simulations per step (e.g. "10k")
    # selection (str) (optional) = selection policy used by agent (default "Greedy Expectimax") 

    df = pd.read_csv(fpath)
    featurize(df)

    df['Heuristic'] = heuristic
    df['Determinizations'] = K
    df['nSims'] = 1000*int(sims[:-1]) if sims.endswith('k') else int(sims if len(sims)>0 else 0)
    df['Selection Policy'] = selection

    return df

def load_dfs(suffix,K,N,heuristic,fpath="./",selection_policy="Greedy Expectimax"):
    # A function to collect and return a concatenated dataframe of a collection of experiments that all share a common suffix
    # Will try collecting ALL of the possible pairs of K and N
    #
    # fpath = relative or absolute filepath to collection of experiment folders (e.g. <fpath>/K1_10k_..../*.csv)
    # suffix = end of experiment-holding folder filename (e.g. <fpath>/K1_10k_<suffix>/*.csv)
    # K = list of ints or strs indicating which values of K to collect (e.g. K=[1,3,5,10,20])
    # N = list of strs representing the number of simulations in the experiment (e.g. N = ["500","10k","50k"])

    dfs = []
    for k in K:
        for n in N:
            folder_name = f"K{k}_{n}_"+suffix+"/"
            full_fpath = os.path.join(fpath,folder_name)
            
            experiment_files = os.listdir(full_fpath)
            if len(experiment_files)==2:

                for exp_file in experiment_files:
                    if exp_file.startswith(f"K{k}_{n}_"):
                        if exp_file.endswith(".csv"):
                            temp_df = load_df(os.path.join(full_fpath,exp_file),k,n,heuristic,selection_policy)
                            dfs.append(temp_df)
                    else:
                        raise Exception
            else:
                raise Exception

    return pd.concat(dfs,axis=0,ignore_index=True)