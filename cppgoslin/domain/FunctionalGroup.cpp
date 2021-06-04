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



FunctionalGroup::FunctionalGroup(FunctionalGroup* fg){
    name = fg->name;
    position = fg->position;
    count = fg->count;
    stereochemistry = fg->stereochemistry;
    ringStereo = fg->ringStereo;
    doubleBonds = new DoubleBonds(fg->doubleBonds);
    
    for (auto &kv : *(fg->elements)) elements->at(kv.first) = kv.second;
    for (auto &kv : *(fg->functionalGroups)){
        functionalGroups->insert({kv.first, vector<FunctionalGroup*>()});
        for (auto fg : kv.second) functionalGroups->at(kv.first).push_back(new FunctionalGroup(fg));
    }
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



FunctionalGroup* FunctionalGroup::getFunctionalGroup(string fgName){
    map<string, FunctionalGroup*>& knownFunctionalGroups = KnownFunctionalGroups::get_instance().knownFunctionalGroups;
    if(contains(knownFunctionalGroups, fgName)){
        return knownFunctionalGroups.at(fgName);
    }
    throw RuntimeException("Name '" + fgName + "' not registered in functional group list");
}




HeadgroupDecorator::HeadgroupDecorator(string _name, int _position, int _count, ElementTable* _elements, bool _suffix, LipidLevel _level) : FunctionalGroup(_name, _position, _count, 0, false, "", _elements){
    suffix = _suffix;
    lowestVisibleLevel = _level;
}


        
        
HeadgroupDecorator::HeadgroupDecorator(HeadgroupDecorator* hgd) : FunctionalGroup(hgd){
    suffix = hgd->suffix;
    lowestVisibleLevel = hgd->lowestVisibleLevel;    
}


string HeadgroupDecorator::toString(LipidLevel level){
    if (!suffix) return name;

    string decoratorString = "";
    if (lowestVisibleLevel == NO_LEVEL || lowestVisibleLevel <= level){
        
        if (contains_p(functionalGroups, "decorator_alkyl") && functionalGroups->at("decorator_alkyl").size() > 0){
            decoratorString = (level != SPECIES) ? functionalGroups->at("decorator_alkyl").at(0)->toString(level) : "Alk";
        }
        else if (contains_p(functionalGroups, "decorator_acyl") && functionalGroups->at("decorator_acyl").size() > 0){
            decoratorString = (level != SPECIES) ? ("FA " + functionalGroups->at("decorator_acyl").at(0)->toString(level)) : "FA";
        }
        else {
            decoratorString = name;
        }
        decoratorString = "(" + decoratorString + ")";
    }
        
    return decoratorString;
}
