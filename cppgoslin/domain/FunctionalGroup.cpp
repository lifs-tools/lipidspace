#include "cppgoslin/domain/FunctionalGroup.h"

FunctionalGroup::FunctionalGroup(string _name, int _position, int _count, DoubleBonds* _doubleBonds, bool _is_atomic, string _stereochemistry, ElementTable* _elements, map<string, vector<FunctionalGroup*>>* _functionalGroups){
    name = _name;
    position = _position;
    count = _count;
    stereochemistry = _stereochemistry;
    ringStereo = "";
    doubleBonds = (_doubleBonds != 0) ? _doubleBonds : new DoubleBonds();
    is_atomic = _is_atomic;
    elements = (_elements != 0) ? _elements : create_empty_table();
    functionalGroups = (_functionalGroups != 0) ? _functionalGroups : (new map<string, vector<FunctionalGroup*>>());
}


FunctionalGroup::~FunctionalGroup(){
    delete doubleBonds;
    delete elements;
    for (auto &kv : *functionalGroups){
        for (auto fg : kv.second){
            delete fg;
        }
    }
    delete functionalGroups;
}


FunctionalGroup* FunctionalGroup::copy(){
    DoubleBonds* db = new DoubleBonds(doubleBonds);
    FunctionalGroup* fg = new FunctionalGroup(name, position, count, db, is_atomic, stereochemistry);
    fg->ringStereo = ringStereo;
    
    for (auto &kv : *elements) fg->elements->at(kv.first) = kv.second;
    for (auto &kv : *functionalGroups){
        fg->functionalGroups->insert({kv.first, vector<FunctionalGroup*>()});
        for (auto fg : kv.second) fg->functionalGroups->at(kv.first).push_back(fg->copy());
    }
    return fg;
}



ElementTable* FunctionalGroup::getElements(){
    computeElements();
    ElementTable* _elements = create_empty_table();
    for (auto &kv : *elements) _elements->at(kv.first) = kv.second;
    ElementTable* fgElements = getFunctionalGroupElements();
    for (auto &kv : *fgElements) _elements->at(kv.first) += kv.second;
    delete fgElements;
    return _elements;
}


void FunctionalGroup::shiftPositions(int shift){
    position += shift;
    for (auto &kv : *functionalGroups){
        for (auto fg : kv.second)
            fg->shiftPositions(shift);
    }
}


ElementTable* FunctionalGroup::getFunctionalGroupElements(){
    ElementTable* _elements = create_empty_table();
    
    for (auto &kv : *functionalGroups){
        for (auto funcGroup : kv.second){
            ElementTable* fgElements = funcGroup->getElements();
            for (auto &el : *fgElements){
                _elements->at(el.first) += el.second * funcGroup->count;
            }
            delete fgElements;
        }
    }
                
    return _elements;
}


void FunctionalGroup::computeElements(){
    for (auto &kv : *functionalGroups){
        for (auto funcGroup : kv.second){
            funcGroup->computeElements();
        }
    }
}



        
string FunctionalGroup::toString(LipidLevel level){
    string fgString = "";
    if (level == ISOMERIC_SUBSPECIES){
        if ('0' <= name[0] && name[0] <= '9'){
            fgString = (position > -1) ? (std::to_string(position) + ringStereo + "(" + name + ")") : name;
        }
        else {
            fgString = (position > -1) ? (std::to_string(position) + ringStereo + name) : name;
        }
    }
    else{
        fgString = (count > 1) ? ("(" + name + ")" + std::to_string(count)) : name;
    }
    if (stereochemistry.length() > 0 && level == ISOMERIC_SUBSPECIES){
        fgString += "[" + stereochemistry + "]";
    }
            
    return fgString;
}


int FunctionalGroup::getDoubleBonds(){
    int db = count * doubleBonds->getNum();
    for (auto &kv : *functionalGroups){
        for (auto funcGroup : kv.second){
            db += funcGroup->getDoubleBonds();
        }
    }
            
    return db;
}




void FunctionalGroup::add(FunctionalGroup* fg){
    for (auto &kv : *(fg->elements)){
        elements->at(kv.first) += kv.second * fg->count;
    }
}
    
