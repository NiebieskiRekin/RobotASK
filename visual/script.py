import pandas as pd
import numpy as np
import math

pomiary = pd.read_csv("test.txt", delimiter=';',header=None,names=["Time","P0","P1","P2","P3","???"],index_col=False)
pomiary = pomiary.drop(columns=["???"])
def chunks(lst, n):
    """Yield successive n-sized chunks from lst."""
    for i in range(0, len(lst), n):
        yield lst[i:i + n]


time_data = [math.floor(np.mean(sublist)) for sublist in chunks(pomiary['Time'],4)]
size = len(time_data)

def up4(n):
    return math.ceil(n/4)*4



# usuń puste pola i *
photores = {"P0":[],"P1":[],"P2":[],"P3":[]}
for n in range(up4(len(pomiary.index))):
    try:
        if "*" == str(pomiary.iloc[n,n%4 + 1][-1]):
            photores[f"P{n%4}"].append(int(pomiary.iloc[n,n%4+1][:-1]))
        else:
            photores[f"P{n%4}"].append(int(pomiary.iloc[n,n%4+1]))
    except IndexError:
        photores[f"P{n%4}"].append(-1)


# # sklej dane
dane = pd.DataFrame({"Time":time_data} | photores)


dane.to_csv('out.csv', index=False)
