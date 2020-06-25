import pandas as pd
import numpy as np

def featurizeEventUsage(df):
    # Instantiate new column for how long airlift was present before use, if used (-1 if either never present or present & not used)
    df['AirliftUseTime']=df.AirliftUse.copy()
    df.AirliftUseTime[df.AirliftUseTime>=0] = df.AirliftUseTime[df.AirliftUseTime>=0] - df.firstAirliftPresence[df.AirliftUseTime>=0]

    # Instantiate new column for how long Government Grant was present before use, if used (-1 if either never present or present & not used)
    df['GovernmentGrantUseTime']=df.GovernmentGrantUse.copy()
    df.GovernmentGrantUseTime[df.GovernmentGrantUseTime>=0] = df.GovernmentGrantUseTime[df.GovernmentGrantUseTime>=0] - df.firstGovernmentGrantPresence[df.GovernmentGrantUseTime>=0]

    # Instantiate new column for how long Quiet Night was present before use, if used (-1 if either never present or present & not used)
    df['QuietNightUseTime']=df.QuietNightUse.copy()
    df.QuietNightUseTime[df.QuietNightUseTime>=0] = df.QuietNightUseTime[df.QuietNightUseTime>=0] - df.firstQuietNightPresence[df.QuietNightUseTime>=0]

def featurizeCure(df):
    df['n_Cured']=1*(df.BlueCured>0) + 1*(df.YellowCured>0) + 1*(df.BlackCured>0) + 1*(df.RedCured>0)

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