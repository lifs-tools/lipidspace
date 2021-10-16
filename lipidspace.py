from pygoslin.parser.Parser import *
from pygoslin.domain.LipidFaBondType import LipidFaBondType
from pygoslin.domain.LipidLevel import LipidLevel
from pygoslin.domain.FunctionalGroup import *
from pygoslin.domain.Element import Element
import sys
from math import sqrt
import pandas as pd
import numpy as np

# load precomputed class distance matrix
parser = LipidParser()
class_matrix = {}
with open("data/classes-matrix.csv") as infile:
    for line in infile:
        tokens = line.strip().split("\t")
        class_matrix["%s/%s" % (tokens[0], tokens[1])] = (abs(int(tokens[2])), abs(int(tokens[3])))
        class_matrix["%s/%s" % (tokens[1], tokens[0])] = (abs(int(tokens[2])), abs(int(tokens[3])))




def fatty_acyl_similarity(fa1, fa2):
    
    inter, union = 0, 0
    
    lcbs = {LipidFaBondType.LCB_REGULAR, LipidFaBondType.LCB_EXCEPTION}
    
    bond_types = set([fa1.lipid_FA_bond_type, fa2.lipid_FA_bond_type])
    if len(bond_types) == 1: # both equal
        if LipidFaBondType.ESTER in bond_types: union += 2 # one oxygen plus one double bond
        if LipidFaBondType.ETHER_PLASMENYL in bond_types: union += 1 # one double bond
        
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
        if key == "cy": raise Exception("Cycles not supported, yet")
            
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
        if key == "cy": raise Exception("Cycles not supported, yet")
    
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
    # load and parse lipids
    lipid_list = [parser.parse(l) for l in open(lipid_list_file, "rt").read().split("\n") if len(l) > 0]
    n = len(lipid_list)
    
    # compute distances
    distance_matrix = [[0] * n for i in range(n)]
    for i in range(n - 1):
        for j in range(i + 1, n):
            union, inter = lipid_similarity(lipid_list[i], lipid_list[j], class_matrix)
            distance = 1 / (inter / union) - 1
            distance_matrix[j][i] = distance_matrix[i][j] = sqrt(distance)
    
    data = {}
    data["ID"]  = ["LP%i" % (i + 1) for i in range(n)]
    data["Species"] = [l.get_lipid_string() for l in lipid_list]
    data["Class"] = [l.get_lipid_string(LipidLevel.CLASS) for l in lipid_list]
    for i, col in enumerate(distance_matrix):
        data["LP%i" % (i + 1)] = col

    return pd.DataFrame(data)

def main(argv):
    
    if len(argv) < 4:
        print("usage: python3 %s output_folder lipid_list[csv], ..." % argv[0])
        print("you need at least two lipidomes")
        exit()
        
    output_folder = argv[1]
    input_lists = argv[2:]
    
    # create all tables
    lipidome_tables = [create_table(input_list) for input_list in input_lists]
    
    # store all tables
    for table, input_list in zip(lipidome_tables, input_lists):
        input_list = input_list.split("/")[-1]
        file_name = "%s.xlsx" % ".".join(input_list.split(".")[:-1]) if input_list.find(".") > -1 else "%s.xlsx" % input_list
        table.to_excel("%s/%s" % (output_folder, file_name), index = False)
    
if __name__ == "__main__":
    main(sys.argv)
