from pygoslin.parser.Parser import *
from pygoslin.domain.FattyAcid import FattyAcid
from pygoslin.domain.LipidFaBondType import LipidFaBondType
from pygoslin.domain.LipidLevel import LipidLevel
from pygoslin.domain.FunctionalGroup import *
from pygoslin.domain.Element import Element
import sys
from math import sqrt
import numpy as np
from sklearn.preprocessing import StandardScaler
import pandas as pd
from sklearn.decomposition import PCA
from sklearn.preprocessing import scale, normalize
import matplotlib.pyplot as plt
import os
from scipy.spatial.distance import directed_hausdorff
from scipy import linalg as LA


script_path = "/".join(os.path.realpath(__file__).split("/")[:-1])


# load precomputed class distance matrix
parser = LipidParser()
class_matrix = {}
with open("%s/data/classes-matrix.csv" % script_path) as infile:
    for line in infile:
        tokens = line.strip().split("\t")
        class_matrix["%s/%s" % (tokens[0], tokens[1])] = (abs(int(tokens[2])), abs(int(tokens[3])))
        class_matrix["%s/%s" % (tokens[1], tokens[0])] = (abs(int(tokens[2])), abs(int(tokens[3])))



def cut_cycle(fa):
    # if a fatty acid contains a cycle, cut everything from the cycle beginning
    
    if "cy" not in fa.functional_groups: return fa

    fa = fa.copy()
    if fa.functional_groups["cy"][0].start > -1:
        start = fa.functional_groups["cy"][0].start
        
        # shrink numbers of carbon
        fa.num_carbon = start - 1
        
        # cut all double bonds
        if type(fa.double_bonds) == int:
            fa.double_bonds = max(fa.double_bonds, (start - 1) // 2)
        else:
            for key in set(fa.double_bonds.keys()):
                if key >= start:
                    del fa.double_bonds[key]
                    
        # cut all functional groups
        del fa.functional_groups["cy"]
        del_fgs = []
        for fg, fg_list in fa.functional_groups.items():
            keep_fg = []
            for func_group in fg_list:
                if 0 <= func_group.position < start:
                    keep_fg.append(func_group)
            if len(keep_fg) > 0:
                fa.functional_groups[fg] = keep_fg
            else:
                del_fgs.append(fg)
                
        for fg in del_fgs: del fa.functional_groups[fg]
        
        return fa
        
    else:
        return FattyAcid("FA", 2, lipid_FA_bond_type = fa.lipid_FA_bond_type)





def fatty_acyl_similarity(fa1, fa2):
    
    inter, union = 0, 0
    fa1, fa2 = cut_cycle(fa1), cut_cycle(fa2)
    
    
    lcbs = {LipidFaBondType.LCB_REGULAR, LipidFaBondType.LCB_EXCEPTION}
    
    bond_types = set([fa1.lipid_FA_bond_type, fa2.lipid_FA_bond_type])
    if len(bond_types) == 1: # both equal
        if LipidFaBondType.ESTER in bond_types:
            union += 2 # one oxygen plus one double bond
            inter += 2
            
        if LipidFaBondType.ETHER_PLASMENYL in bond_types:
            union += 1 # one double bond
            union += 1
        
    elif LipidFaBondType.ESTER in bond_types and LipidFaBondType.ETHER_PLASMENYL in bond_types:
        union += 3
        
    elif LipidFaBondType.ESTER in bond_types and LipidFaBondType.ETHER_PLASMANYL in bond_types:
        union += 2
        
    elif LipidFaBondType.ETHER_PLASMENYL in bond_types and LipidFaBondType.ETHER_PLASMANYL in bond_types:
        union += 1
            
    elif LipidFaBondType.LCB_REGULAR in bond_types and LipidFaBondType.ESTER in bond_types:
        union += 2
            
    elif LipidFaBondType.LCB_EXCEPTION in bond_types and LipidFaBondType.ESTER in bond_types:
        union += 2
            
    elif LipidFaBondType.LCB_REGULAR in bond_types and LipidFaBondType.ETHER_PLASMANYL in bond_types:
        pass
            
    elif LipidFaBondType.LCB_EXCEPTION in bond_types and LipidFaBondType.ETHER_PLASMANYL in bond_types:
        pass
            
    elif LipidFaBondType.LCB_REGULAR in bond_types and LipidFaBondType.ETHER_PLASMENYL in bond_types:
        union += 1
            
    elif LipidFaBondType.LCB_EXCEPTION in bond_types and LipidFaBondType.ETHER_PLASMENYL in bond_types:
        union += 1
            
    elif LipidFaBondType.LCB_REGULAR in bond_types and LipidFaBondType.LCB_EXCEPTION in bond_types:
        pass
        
    else: 
        raise Exception("Cannot compute similarity between %s and %s chains" % (fa1.lipid_FA_bond_type, fa2.lipid_FA_bond_type))
    
    # compare lengths
    m1 = 2 if fa1.lipid_FA_bond_type in lcbs else 0
    m2 = 2 if fa2.lipid_FA_bond_type in lcbs else 0
    inter += min(fa1.num_carbon - m1, fa2.num_carbon - m1)
    union += max(fa1.num_carbon - m2, fa2.num_carbon - m2)
    
    
    # compare double bonds
    if type(fa1.double_bonds) == dict and type(fa2.double_bonds) == dict:
        common = 0
        fa1_keys = set(fa1.double_bonds.keys())
        for key in fa1_keys:
            common += 1 if key in fa2.double_bonds and fa1.double_bonds[key] == fa2.double_bonds[key] else 0
        inter += common
        union += len(fa1.double_bonds) + len(fa2.double_bonds) - common
        db1, db2 = len(fa1.double_bonds), len(fa2.double_bonds)
    
    else:
        db1, db2 = 0, 0
        if type(fa1.double_bonds) == dict:
            db1, db2 = len(fa1.double_bonds), fa2.double_bonds
            
        elif type(fa2.double_bonds) == dict:
            db1, db2 = fa1.double_bonds, len(fa2.double_bonds)
            
        else:
            db1, db2 = fa1.double_bonds, fa2.double_bonds
            
        inter += min(db1, db2)
        union += max(db1, db2)
    
    
    # add all single bonds
    inter += min(fa1.num_carbon - m1 - db1, fa2.num_carbon - m1 - db2)
    union += max(fa1.num_carbon - m2 - db1, fa2.num_carbon - m2 - db2)
    
        
    # compare functional groups
    for key in fa1.functional_groups.keys():
        if key == "[X]": continue
            
        func_group = get_functional_group(key)
        elements = sum([v for k, v in func_group.get_elements().items() if k != Element.H]) + func_group.get_double_bonds()
        
        if key in fa2.functional_groups: # functional group occurs in both fatty acids
            
            # both func groups contain position information
            if fa1.functional_groups[key][0].position > -1 and fa2.functional_groups[key][0].position > -1:
                keys_fa1 = set([f.position for f in fa1.functional_groups[key]])
                keys_fa2 = set([f.position for f in fa2.functional_groups[key]])
                
                inter += len(keys_fa1 & keys_fa2)
                union += len(keys_fa1 | keys_fa2)
                
        else:
            num = len(fa1.functional_groups[key]) if fa1.functional_groups[key][0].position > -1 else sum([f.count for f in fa1.functional_groups[key]])
            union += elements * num
            
    for key in (fa2.functional_groups.keys() - fa1.functional_groups.keys()):
        if key == "[X]": continue
    
        func_group = get_functional_group(key)
        elements = sum([v for k, v in func_group.get_elements().items() if k != Element.H]) + func_group.get_double_bonds()
        
        num = len(fa2.functional_groups[key]) if fa2.functional_groups[key][0].position > -1 else sum([f.count for f in fa2.functional_groups[key]])
        union += elements * num
            
    return union, inter







def lipid_similarity(lipid1, lipid2, class_matrix):
    key = "%s/%s" % (lipid1.get_extended_class(), lipid2.get_extended_class())
    union, inter = class_matrix[key]
    
    
    for fa1, fa2 in zip(lipid1.lipid.fa_list, lipid2.lipid.fa_list):
        u, i = fatty_acyl_similarity(fa1, fa2)
        union += u
        inter += i
    
    
    fa_list_1, fa_list_2 = lipid1.lipid.fa_list, lipid2.lipid.fa_list
    if len(fa_list_1) < len(fa_list_2):
        fa_list_1, fa_list_2 = fa_list_2, fa_list_1
        
    if len(fa_list_1) > len(fa_list_2):
        for i in range(len(fa_list_2), len(fa_list_1)):
            fa = fa_list_1[i]
            num = sum(v for k, v in fa.get_elements().items() if k != Element.H) + fa.get_double_bonds()
            union += num
    
    return union, inter







def create_table(lipid_list_file):
    print("reading '%s'" % lipid_list_file)
    
    # load and parse lipids
    #lipid_list = [parser.parse(l.strip('"')) for l in open(lipid_list_file, "rt").read().split("\n") if len(l) > 0]
    lipid_list = []
    for l in open(lipid_list_file, "rt").read().split("\n"):
        if len(l) > 0:
            try:
                l = l.split("\t")[0].strip('"')
                lipid_list.append(parser.parse(l))
                                  
            except Exception:
                print("Error: lipid '%s' cannot be parsed" % l)
                exit()
    
    
    n = len(lipid_list)
    
    # compute distances
    distance_matrix = [[0] * n for i in range(n)]
    for i in range(n - 1):
        for j in range(i + 1, n):
            union, inter = lipid_similarity(lipid_list[i], lipid_list[j], class_matrix)
            distance = 1 / (inter / union) - 1
            distance_matrix[j][i] = distance_matrix[i][j] = distance #sqrt(distance)
    
    
    data = {}
    data["ID"]  = ["LP%i" % (i + 1) for i in range(n)]
    data["Species"] = [l.get_lipid_string() for l in lipid_list]
    data["Class"] = [l.get_lipid_string(LipidLevel.CLASS) for l in lipid_list]
    for i, col in enumerate(distance_matrix):
        data["LP%i" % (i + 1)] = col

    return pd.DataFrame(data)






def compute_PCA(data_frame):
    features = [l for l in data_frame if l[:2] == "LP"]
    x = data_frame.loc[:, features].values# Separating out the target
    x = StandardScaler().fit_transform(x)
    pca = PCA(n_components = min(100, len(x)))
    return (pca, pca.fit_transform(x))






def compute_hausdorff_matrix(PCAs, lipid_lists):
    n = len(lipid_lists)
    distance_matrix = [[0] * n for i in range(n)]
    
    for i in range(n - 1):
        for j in range(i + 1, n):
            m1, m2 = PCAs[i][1][:,:2], PCAs[j][1][:,:2]
            h = max(directed_hausdorff(m1, m2)[0], directed_hausdorff(m2, m1)[0])
            distance_matrix[j][i] = distance_matrix[i][j] = sqrt(h)
    
    lipidomes = [ll.split("/")[-1] for ll in lipid_lists]
    data = {"ID": lipidomes}
    for ll, col in zip(lipidomes, distance_matrix): data[ll] = col
    return pd.DataFrame(data)
    


def main(argv):
    
    plot_pca = False
    store_tables = False
    
    if len(argv) < 3:
        print("usage: python3 %s output_folder lipid_list[csv], ..." % argv[0])
        exit()
        
    output_folder = argv[1]
    input_lists = argv[2:]
    
    # create all tables
    lipidome_tables = [create_table(input_list) for input_list in input_lists]
    
    # store all tables
    if store_tables:
        for table, input_list in zip(lipidome_tables, input_lists):
            input_list = input_list.split("/")[-1]
            file_name = "%s.xlsx" % ".".join(input_list.split(".")[:-1]) if input_list.find(".") > -1 else "%s.xlsx" % input_list
            file_name = "%s/%s" % (output_folder, file_name)
            print("storing '%s'" % file_name)
            table.to_excel(file_name, index = False)

    
    # compute all PCAs
    print("computing principal components for all tables")
    PCAs = [compute_PCA(table) for table in lipidome_tables]
    
    
    # plot all PCAs
    if plot_pca:
        print("storing principal components for all tables")
        for pca, df, input_list in zip(PCAs, lipidome_tables, input_lists):
            principalDf = pd.DataFrame(data = pca[1][:,:2], columns = ['principal component 1', 'principal component 2'])
            finalDf = pd.concat([principalDf, df[['Class']]], axis = 1)
            
            fig = plt.figure(figsize = (8, 8))
            ax = fig.add_subplot(1, 1, 1) 
            ax.set_xlabel('Principal Component 1 (%0.1f %%)' % (pca[0].explained_variance_ratio_[0] * 100), fontsize = 15)
            ax.set_ylabel('Principal Component 2 (%0.1f %%)' % (pca[0].explained_variance_ratio_[1] * 100), fontsize = 15)
            ax.set_title("'%s' PCA" % input_list, fontsize = 20)

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
            
            input_list = input_list.split("/")[-1]
            file_name = "%s.pdf" % ".".join(input_list.split(".")[:-1]) if input_list.find(".") > -1 else "%s.pdf" % input_list
            file_name = "%s/%s" % (output_folder, file_name)
            print("storing '%s'" % file_name)
            plt.savefig(file_name, dpi = 300)
    
    
    if len(input_lists) == 1: return
    
    
    # create hausdorff distance matrix
    hausdorff_distances = compute_hausdorff_matrix(PCAs, input_lists)
    hausdorff_distances.to_excel("%s/hausdorff_distances.xlsx" % output_folder, index = False)
    
    
if __name__ == "__main__":
    main(sys.argv)
