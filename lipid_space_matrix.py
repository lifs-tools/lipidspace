from pygoslin.parser.Parser import *
from pygoslin.domain.LipidFaBondType import LipidFaBondType
from pygoslin.domain.LipidLevel import LipidLevel
from pygoslin.domain.FunctionalGroup import *
from pygoslin.domain.Element import Element
import sys

WEIGHT_ATOM_DIFFER = 1
WEIGHT_BOND_DIFFER = 1

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




def main(argv):
    
    if len(argv) < 3:
        print("usage: python3 %s lipid_list[csv] output_matrix[csv]" % argv[0])
        exit()
        
    input_list = argv[1]
    output_matrix = argv[2]

    # load precomputed class distance matrix
    class_matrix = {}
    with open("classes-matrix.csv") as infile:
        for line in infile:
            tokens = line.strip().split("\t")
            class_matrix["%s/%s" % (tokens[0], tokens[1])] = (abs(int(tokens[2])), abs(int(tokens[3])))
            class_matrix["%s/%s" % (tokens[1], tokens[0])] = (abs(int(tokens[2])), abs(int(tokens[3])))

    # load and parse lipids
    parser = GoslinParser()
    lipid_list = [parser.parse(l) for l in open(input_list, "rt").read().split("\n") if len(l) > 0]
    n = len(lipid_list)
    
    # compute distances
    distance_matrix = [[0] * n for i in range(n)]
    for i in range(n - 1):
        for j in range(i + 1, n):
            union, inter = lipid_similarity(lipid_list[i], lipid_list[j], class_matrix)
            distance = 1 / (inter / union) - 1
            distance_matrix[i][j] = distance
            distance_matrix[j][i] = distance
    
    with open(output_matrix, "wt") as out:
        out.write("ID\tName\tClass\t%s\n" % "\t".join("LP%i" % (i + 1) for i in range(n)))
        for i in range(n):
            out.write("LP%i\t%s\t%s" % ((i + 1), lipid_list[i].get_lipid_string(), lipid_list[i].get_lipid_string(LipidLevel.CLASS)))
            out.write("\t%s\n" % "\t".join("%0.4f" % d for d in distance_matrix[i]))
    
if __name__ == "__main__":
    main(sys.argv)
