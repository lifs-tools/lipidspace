from sklearn.preprocessing import StandardScaler
import pandas as pd
from sklearn.decomposition import PCA
import matplotlib.pyplot as plt
import sys

if len(sys.argv) < 3:
    print("usage: python3 %s input_matrix[csv] output_figure[pdf]" % argv[0])
    exit()
    

input_matrix = sys.argv[1]
output_figure = sys.argv[2]

df = pd.read_csv(input_matrix, sep = "\t")
features = [l for l in df if l[:2] == "LP"]
x = df.loc[:, features].values# Separating out the target
y = df.loc[:,['Class']].values# Standardizing the features
x = StandardScaler().fit_transform(x)


pca = PCA(n_components = 2)
principalComponents = pca.fit_transform(x)
principalDf = pd.DataFrame(data = principalComponents, columns = ['principal component 1', 'principal component 2'])
finalDf = pd.concat([principalDf, df[['Class']]], axis = 1)

fig = plt.figure(figsize = (8,8))
ax = fig.add_subplot(1,1,1) 
ax.set_xlabel('Principal Component 1', fontsize = 15)
ax.set_ylabel('Principal Component 2', fontsize = 15)
ax.set_title('2 component PCA', fontsize = 20)

targets = set(df["Class"])
#for target, color in zip(targets, colors):
for target in targets:
    indicesToKeep = finalDf['Class'] == target
    #if target != "PC": continue
    ax.scatter(finalDf.loc[indicesToKeep, 'principal component 1'],
               finalDf.loc[indicesToKeep, 'principal component 2'],
               label = target)
    
    
    
ax.legend()
ax.grid()
plt.savefig(output_figure, dpi = 300)
