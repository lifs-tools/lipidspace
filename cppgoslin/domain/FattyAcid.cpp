/*
MIT License

Copyright (c) 2020 Dominik Kopczynski   -   dominik.kopczynski {at} isas.de
                   Nils Hoffmann  -  nils.hoffmann {at} isas.de

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include "FattyAcid.h"

FattyAcid::FattyAcid(string _name, int _num_carbon, int _num_double_bonds, int _num_hydroxyl, LipidFaBondType _lipid_FA_bond_type, bool _lcb, int _position, map<int, string> *_double_bond_positions){
    name = _name;
    position = _position;
    num_carbon = _num_carbon;
    num_double_bonds = _num_double_bonds;
    num_hydroxyl = _num_hydroxyl;
    lipid_FA_bond_type = _lipid_FA_bond_type;
    lcb = _lcb;
    if (_double_bond_positions != NULL){
        if (num_double_bonds != (int)_double_bond_positions->size()) {
            throw ConstraintViolationException("Isomeric FattyAcid must receive double bond positions for all double bonds! Got " + std::to_string(num_double_bonds) + " double bonds and " + std::to_string(_double_bond_positions->size()) + " positions.");
        }
        for (auto kv : *_double_bond_positions){
            double_bond_positions.insert({kv.first, kv.second});
        }
    }

    
    if (num_carbon < 2){
        throw ConstraintViolationException("FattyAcid must have at least 2 carbons! Got " + std::to_string(num_carbon));
    }
    
    if (position < 0){
        throw ConstraintViolationException("FattyAcid position must be greater or equal to 0! Got " + std::to_string(position));
    }
    
    if (num_double_bonds < 0){
        throw ConstraintViolationException("FattyAcid must have at least 0 double bonds! Got " + std::to_string(num_double_bonds));
    }
    
    if (num_hydroxyl < 0){
        throw ConstraintViolationException("FattyAcid must have at least 0 hydroxy groups! Got " + std::to_string(num_hydroxyl));
    }
}


FattyAcid::FattyAcid(){
    name = "";
    position = 0;
    num_carbon = 0;
    num_double_bonds = 0;
    num_hydroxyl = 0;
    lipid_FA_bond_type = ESTER;
    lcb = false;
}


FattyAcid::FattyAcid(FattyAcid* fa){
    if (fa) {
        name = fa->name;
        position = fa->position;
        num_carbon = fa->num_carbon;
        num_hydroxyl = fa->num_hydroxyl;
        num_double_bonds = fa->num_double_bonds;
        lipid_FA_bond_type = fa->lipid_FA_bond_type;
        lcb = fa->lcb;
        for (auto kv : fa->double_bond_positions){
            double_bond_positions.insert({kv.first, kv.second});
        }
    }
    else {
        name = "";
        position = 0;
        num_carbon = 0;
        num_double_bonds = 0;
        num_hydroxyl = 0;
        lipid_FA_bond_type = ESTER;
        lcb = false;
    }
}

string FattyAcid::suffix(LipidFaBondType lipid_FA_bond_type){
    switch(lipid_FA_bond_type){
        case (ETHER_PLASMANYL): return "a";
        case (ETHER_PLASMENYL): return "p";
        default: return "";
    }
}


string FattyAcid::to_string(bool special_case){
    stringstream s;
    string lipid_suffix = suffix(lipid_FA_bond_type);
    if (special_case && lipid_suffix.length() > 0) s << "O-";
    s << num_carbon << ":" << num_double_bonds;
    
    if (double_bond_positions.size() > 0 && num_double_bonds != (int)double_bond_positions.size()) {
        throw ConstraintViolationException("Isomeric FattyAcid must receive double bond positions for all double bonds! Got " + std::to_string(num_double_bonds) + " double bonds and " + std::to_string(double_bond_positions.size()) + " positions.");
    }
    
    if (double_bond_positions.size()){
        stringstream db;
        db << "(";
        int j = 0;
        for (auto it : double_bond_positions){
            if (j > 0) db << ",";
            db << it.first << it.second;
            ++j;
        }
        db << ")";
        s << db.str();
    }
    
    if (num_hydroxyl) s << ";" << num_hydroxyl;
    s << lipid_suffix;
    return s.str();
}


ElementTable* FattyAcid::get_elements(){
    ElementTable* table = create_empty_table();
    
    if (!lcb){ 
        
        if (num_carbon > 0 || num_double_bonds > 0){
            table->at(ELEMENT_C) = num_carbon; // C
            switch(lipid_FA_bond_type)
            {
                case ESTER:
                    table->at(ELEMENT_H) = (2 * num_carbon - 1 - 2 * num_double_bonds); // H
                    table->at(ELEMENT_O) = (1 + num_hydroxyl); // O
                    break;
                case ETHER_PLASMENYL:
                    table->at(ELEMENT_H) = (2 * num_carbon - 1 - 2 * num_double_bonds + 2); // H
                    table->at(ELEMENT_O) = num_hydroxyl; // O
                    break;
                case ETHER_PLASMANYL:
                    table->at(ELEMENT_H) = ((num_carbon + 1) * 2 - 1 - 2 * num_double_bonds); // H
                    table->at(ELEMENT_O) = num_hydroxyl; // O
                    break;
                default:
                    throw LipidException("Mass cannot be computed for fatty acyl chain with bond type: " + std::to_string(lipid_FA_bond_type));
            }
        }
    }
    else 
    {
        // long chain base
        table->at(ELEMENT_C) = num_carbon; // C
        table->at(ELEMENT_H) = (2 * (num_carbon - num_double_bonds) + 1); // H
        table->at(ELEMENT_O) = num_hydroxyl; // O
        table->at(ELEMENT_N) = 1; // N
    }
    
    return table;
}
