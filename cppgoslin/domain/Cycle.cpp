#include "cppgoslin/domain/Cycle.h"

Cycle::Cycle(int _cycle, int _start, int _end, DoubleBonds* _doubleBonds, map<string, vector<FunctionalGroup*> >* _functionalGroups, vector<Element>* _bridgeChain) : FunctionalGroup("cy", -1, 1, _doubleBonds, false, "", 0, _functionalGroups){
    count = 1;
    cycle = _cycle;
    position = _start;
    start = _start;
    end = _end;
    elements->at(ELEMENT_H) = -2;
    bridgeChain = (_bridgeChain == 0) ? new vector<Element>() : _bridgeChain;
}
        

Cycle::Cycle(Cycle* c) : FunctionalGroup(c){
    cycle = c->cycle;
    start = c->start;
    end = c->end;
    elements->at(ELEMENT_H) = -2;
    bridgeChain = new vector<Element>();
    for (auto &chainElement : *(c->bridgeChain)) bridgeChain->push_back(chainElement);
}
                
                
int Cycle::getDoubleBonds(){
    return FunctionalGroup::getDoubleBonds() + 1;
}
    
    
void Cycle::rearrangeFunctionalGroups(FunctionalGroup* parent, int shift){
    // put everything back into parent
    for (auto &kv : doubleBonds->doubleBondPositions) {
        parent->doubleBonds->doubleBondPositions.insert({kv.first, kv.second});
    }
    delete doubleBonds;
    doubleBonds = new DoubleBonds();
    
    for (auto &kv : *functionalGroups){
        if (!contains_p(parent->functionalGroups, kv.first)){
            parent->functionalGroups->insert({kv.first, vector<FunctionalGroup*>()});
        }
        parent->functionalGroups->at(kv.first).insert(parent->functionalGroups->at(kv.first).end(), functionalGroups->at(kv.first).begin(), functionalGroups->at(kv.first).end());
    }
    delete functionalGroups;
    functionalGroups = new map<string, vector<FunctionalGroup*> >();
    
    
    // shift the cycle
    shiftPositions(shift);
    
    
    // take back what's mine# check double bonds
    for (auto &kv : parent->doubleBonds->doubleBondPositions){
        if (start <= kv.first && kv.first <= end){
            doubleBonds->doubleBondPositions.insert({kv.first, kv.second});
        }
    }
    doubleBonds->numDoubleBonds = doubleBonds->doubleBondPositions.size();
    
    for (auto &kv : doubleBonds->doubleBondPositions){
        parent->doubleBonds->doubleBondPositions.erase(kv.first);
    }
    parent->doubleBonds->numDoubleBonds = parent->doubleBonds->doubleBondPositions.size();
    
            
    set<string> removeList;
    for (auto &kv : *(parent->functionalGroups)){
        vector<int> removeItem;
        
        int i = 0;
        for (auto funcGroup : kv.second){
            if (start <= funcGroup->position && funcGroup->position != end && funcGroup != this){
                if (!contains_p(functionalGroups, kv.first)){
                    functionalGroups->insert({kv.first, vector<FunctionalGroup*>()});
                }
                functionalGroups->at(kv.first).push_back(funcGroup);
                removeItem.push_back(i);
            }
            ++i;
        }
                
        while (!removeItem.empty()){
            int pos = removeItem.back();
            removeItem.pop_back();
            kv.second.erase(kv.second.begin() + pos);
        }
        if (kv.second.empty()) removeList.insert(kv.first);
    }
        
    for (string fg : removeList) parent->functionalGroups->erase(fg);
}    
        
void Cycle::shiftPositions(int shift){
    FunctionalGroup::shiftPositions(shift);
    start += shift;
    end += shift;
    DoubleBonds* db = new DoubleBonds();
    for (auto &kv : doubleBonds->doubleBondPositions) db->doubleBondPositions.insert({kv.first, kv.second});
    doubleBonds->numDoubleBonds = doubleBonds->doubleBondPositions.size();
}
    

void Cycle::computeElements(){
    ElementTable* elements = create_empty_table();
    elements->at(ELEMENT_H) = -2 - 2 * doubleBonds->numDoubleBonds;
    
    for (auto &chainElement : *bridgeChain){
        switch(chainElement){
            case ELEMENT_C:
                elements->at(ELEMENT_C) += 1;
                elements->at(ELEMENT_H) += 2;
                break;
                
            case ELEMENT_N:
                elements->at(ELEMENT_N) += 1;
                elements->at(ELEMENT_H) += 1;
                break;
                
            case ELEMENT_O:
                elements->at(ELEMENT_O) += 1;
                break;
                
            default:
                break;
        }
        
    }
        
    // add all implicit carbon chain elements
    if (start != -1 && end != -1){
        int n = cycle - (end - start + 1 + bridgeChain->size());
        elements->at(ELEMENT_C) += n;
        elements->at(ELEMENT_H) += n << 1;
    }
}
        
bool Cycle::sortFunction (FunctionalGroup* f1, FunctionalGroup *f2) {
    return (f1->position < f2->position);
}
    
string Cycle::toString(LipidLevel level){
    stringstream cycleString;
    cycleString << "[";
    if (start != -1 && level == ISOMERIC_SUBSPECIES){
        cycleString << start << "-" << end;
    }
    
    if ((level == ISOMERIC_SUBSPECIES || level == STRUCTURAL_SUBSPECIES) && bridgeChain->size() > 0){
        cycleString << "(";
        for (auto &e : *bridgeChain) cycleString << element_shortcut.at(e);
        cycleString << ")";
    }
    cycleString << "cy" << cycle;
    
    cycleString << ":" << doubleBonds->numDoubleBonds;
    
    if (doubleBonds->doubleBondPositions.size() > 0){
        int i = 0;
        cycleString << "(";
        for (auto &kv : doubleBonds->doubleBondPositions){
            if (i++ > 0) cycleString << ",";
            cycleString << kv.first << kv.second;
        }
        cycleString << ")";
    }
    
    if (level == ISOMERIC_SUBSPECIES){
        vector<string> fgNames;
        for (auto &kv : *functionalGroups) fgNames.push_back(toLower(kv.first));
        sort(fgNames.begin(), fgNames.end());
        
        for (auto &fg : fgNames){
            vector<FunctionalGroup*>& fgList = functionalGroups->at(fg);
            if (fgList.empty()) continue;
            
            sort(fgList.begin(), fgList.end(), sortFunction);
            int i = 0;
            cycleString << ";";
            for (auto funcGroup : fgList){
                if (i++ > 0) cycleString << ",";
                cycleString << funcGroup->toString(level);
            }
        }
    }
    
    else if (level == STRUCTURAL_SUBSPECIES){
        vector<string> fgNames;
        for (auto &kv : *functionalGroups) fgNames.push_back(toLower(kv.first));
        sort(fgNames.begin(), fgNames.end());
        
        for (auto &fg : fgNames){
            vector<FunctionalGroup*> &fgList = functionalGroups->at(fg);
            if (fgList.empty()) continue;
            
            else if (fgList.size() == 1){
                cycleString << ";" << fgList.front()->toString(level);
            }
            else {
                int fgCount = 0;
                for (auto funcGroup : fgList) fgCount += funcGroup->count;
                if (fgCount > 1){
                    cycleString << ";(" << fg << ")" << fgCount;
                }
                    else{ cycleString << ";" << fg;
                }
            }
        }
    }
                
    cycleString << "]";
    if (stereochemistry.length() > 0) cycleString << "[" << stereochemistry << "]";
    
    return cycleString.str();
}
