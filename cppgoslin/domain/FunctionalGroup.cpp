#include "cppgoslin/domain/FunctionalGroup.h"

FunctionalGroup::FunctionalGroup(string _name, int _position, int _count, DoubleBonds* _double_bonds, bool _is_atomic, string _stereochemistry, ElementTable* _elements, map<string, vector<FunctionalGroup*>>* _functional_groups){
    name = _name;
    position = _position;
    count = _count;
    stereochemistry = _stereochemistry;
    ring_stereo = "";
    double_bonds = (_double_bonds != 0) ? _double_bonds : new DoubleBonds();
    is_atomic = _is_atomic;
    elements = (_elements != 0) ? _elements : create_empty_table();
    functional_groups = (_functional_groups != 0) ? _functional_groups : (new map<string, vector<FunctionalGroup*>>());
}



bool FunctionalGroup::position_sort_function (FunctionalGroup* f1, FunctionalGroup *f2) {
    return (f1->position < f2->position);
}


bool FunctionalGroup::lower_name_sort_function (string s1, string s2) {
    return (to_lower(s1) < to_lower(s2));
}



FunctionalGroup::FunctionalGroup(FunctionalGroup* fg){
    name = fg->name;
    position = fg->position;
    count = fg->count;
    stereochemistry = fg->stereochemistry;
    ring_stereo = fg->ring_stereo;
    double_bonds = new DoubleBonds(fg->double_bonds);
    elements = create_empty_table();
    functional_groups = new map<string, vector<FunctionalGroup*>>();
    is_atomic = fg->is_atomic;
    
    for (auto &kv : *(fg->elements)) elements->at(kv.first) = kv.second;
    for (auto &kv : *(fg->functional_groups)){
        functional_groups->insert({kv.first, vector<FunctionalGroup*>()});
        for (auto func_group : kv.second) functional_groups->at(kv.first).push_back(new FunctionalGroup(func_group));
    }
}



FunctionalGroup::~FunctionalGroup(){
    delete double_bonds;
    delete elements;
    for (auto &kv : *functional_groups){
        for (auto fg : kv.second){
            delete fg;
        }
    }
    delete functional_groups;
}



ElementTable* FunctionalGroup::get_elements(){
    compute_elements();
    ElementTable* _elements = create_empty_table();
    for (auto &kv : *elements) _elements->at(kv.first) = kv.second;
    ElementTable* fgElements = get_functional_group_elements();
    for (auto &kv : *fgElements) _elements->at(kv.first) += kv.second;
    delete fgElements;
    return _elements;
}


void FunctionalGroup::shift_positions(int shift){
    position += shift;
    for (auto &kv : *functional_groups){
        for (auto fg : kv.second)
            fg->shift_positions(shift);
    }
}


ElementTable* FunctionalGroup::get_functional_group_elements(){
    ElementTable* _elements = create_empty_table();
    
    for (auto &kv : *functional_groups){
        for (auto func_group : kv.second){
            ElementTable* fg_elements = func_group->get_elements();
            for (auto &el : *fg_elements){
                _elements->at(el.first) += el.second * func_group->count;
            }
            delete fg_elements;
        }
    }
                
    return _elements;
}


void FunctionalGroup::compute_elements(){
    for (auto &kv : *functional_groups){
        for (auto func_group : kv.second){
            func_group->compute_elements();
        }
    }
}



        
string FunctionalGroup::to_string(LipidLevel level){
    string fg_string = "";
    if (level == ISOMERIC_SUBSPECIES){
        if ('0' <= name[0] && name[0] <= '9'){
            fg_string = (position > -1) ? (std::to_string(position) + ring_stereo + "(" + name + ")") : name;
        }
        else {
            fg_string = (position > -1) ? (std::to_string(position) + ring_stereo + name) : name;
        }
    }
    else{
        fg_string = (count > 1) ? ("(" + name + ")" + std::to_string(count)) : name;
    }
    if (stereochemistry.length() > 0 && level == ISOMERIC_SUBSPECIES){
        fg_string += "[" + stereochemistry + "]";
    }
            
    return fg_string;
}


int FunctionalGroup::get_double_bonds(){
    int db = count * double_bonds->get_num();
    for (auto &kv : *functional_groups){
        for (auto func_group : kv.second){
            db += func_group->get_double_bonds();
        }
    }
            
    return db;
}




void FunctionalGroup::add(FunctionalGroup* fg){
    for (auto &kv : *(fg->elements)){
        elements->at(kv.first) += kv.second * fg->count;
    }
}



FunctionalGroup* FunctionalGroup::get_functional_group(string fg_name){
    map<string, FunctionalGroup*>& known_functional_groups = KnownFunctionalGroups::get_instance().known_functional_groups;
    if(contains(known_functional_groups, fg_name)){
        return new FunctionalGroup(known_functional_groups.at(fg_name));
    }
    throw RuntimeException("Name '" + fg_name + "' not registered in functional group list");
}




HeadgroupDecorator::HeadgroupDecorator(string _name, int _position, int _count, ElementTable* _elements, bool _suffix, LipidLevel _level) : FunctionalGroup(_name, _position, _count, 0, false, "", _elements){
    suffix = _suffix;
    lowest_visible_level = _level;
}


        
        
HeadgroupDecorator::HeadgroupDecorator(HeadgroupDecorator* hgd) : FunctionalGroup(hgd){
    suffix = hgd->suffix;
    lowest_visible_level = hgd->lowest_visible_level;    
}


string HeadgroupDecorator::to_string(LipidLevel level){
    if (!suffix) return name;

    string decorator_string = "";
    if (lowest_visible_level == NO_LEVEL || lowest_visible_level <= level){
        
        if (contains_p(functional_groups, "decorator_alkyl") && functional_groups->at("decorator_alkyl").size() > 0){
            decorator_string = (level != SPECIES) ? functional_groups->at("decorator_alkyl").at(0)->to_string(level) : "Alk";
        }
        else if (contains_p(functional_groups, "decorator_acyl") && functional_groups->at("decorator_acyl").size() > 0){
            decorator_string = (level != SPECIES) ? ("FA " + functional_groups->at("decorator_acyl").at(0)->to_string(level)) : "FA";
        }
        else {
            decorator_string = name;
        }
        decorator_string = "(" + decorator_string + ")";
    }
        
    return decorator_string;
}
