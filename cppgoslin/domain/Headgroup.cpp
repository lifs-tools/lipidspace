#include "cppgoslin/domain/Headgroup.h"
    
Headgroup::Headgroup(string _headgroup, vector<HeadgroupDecorator*>* _decorators, bool _use_headgroup){
    headgroup = _headgroup;
    lipid_category = get_category(_headgroup);
    lipid_class = get_class(headgroup);
    use_headgroup = _use_headgroup;
    decorators = (_decorators != 0) ? _decorators : new vector<HeadgroupDecorator*>();
    sp_exception = lipid_category == SP && ((LipidClasses::get_instance().lipid_classes.at(lipid_class).class_name.compare("Cer") != 0 && LipidClasses::get_instance().lipid_classes.at(lipid_class).class_name.compare("SPB") != 0) || decorators->size() > 0);
}


Headgroup::~Headgroup(){
    for (auto hgd : *decorators) delete hgd;
    delete decorators;
}
        

LipidCategory Headgroup::get_category(string _headgroup){
    if (!StringCategory.size()){
        for (const auto& kvp : LipidClasses::get_instance().lipid_classes){
            LipidCategory category = kvp.second.lipid_category;
            for (auto hg : kvp.second.synonyms){
                StringCategory.insert(pair<string, LipidCategory>(hg, category));
            }
        }
    }

    auto cat = StringCategory.find(_headgroup);
    return (cat != StringCategory.end()) ? StringCategory.at(_headgroup) : UNDEFINED;
}



LipidClass Headgroup::get_class(string _headgroup){
    if (!StringClass.size()){
        for (auto kvp : LipidClasses::get_instance().lipid_classes){
            LipidClass l_class = kvp.first;
            for (auto hg : kvp.second.synonyms){
                StringClass.insert({hg, l_class});
            }
        }
    }
    
    auto cl = StringClass.find(_headgroup);
    return (cl != StringClass.end()) ? cl->second : UNDEFINED_CLASS;
}


string Headgroup::get_class_string(LipidClass _lipid_class){
    if (!ClassString.size()){
        for (auto kvp : LipidClasses::get_instance().lipid_classes){
            ClassString.insert({kvp.first, kvp.second.synonyms.at(0)});
        }
    }
    auto cl = ClassString.find(_lipid_class);
    return (cl != ClassString.end()) ? ClassString.at(_lipid_class) : "UNDEFINED";
}


string Headgroup::get_class_name(){
    return LipidClasses::get_instance().lipid_classes.at(lipid_class).class_name;
}


string Headgroup::get_category_string(LipidCategory _lipid_category){
    return CategoryString.at(_lipid_category);
}        
        
        
string Headgroup::get_lipid_string(LipidLevel level){
    if (level == CATEGORY){
        return get_category_string(lipid_category);
    }
    
    string hgs = ((use_headgroup) ? headgroup : get_class_string(lipid_class));
    
    if (level == CLASS){
        return hgs;
    }
    
    stringstream headgoup_string;
            
    // adding prefixes to the headgroup
    if (level != ISOMERIC_SUBSPECIES && level != STRUCTURAL_SUBSPECIES){
        vector<string> prefixes;
        for (auto hgd : *decorators){
            if (!hgd->suffix) prefixes.push_back(hgd->to_string(level));
        }
        sort (prefixes.begin(), prefixes.end());
        for (auto prefix : prefixes) headgoup_string << prefix;
    }
    else {
        for (auto hgd : *decorators){
            if (!hgd->suffix) headgoup_string << hgd->to_string(level) << "-";
        }
    }
        
    // adding headgroup
    headgoup_string << hgs;
        
    // ading suffixes to the headgroup
    for (auto hgd : *decorators){
        if (hgd->suffix) headgoup_string << hgd->to_string(level);
    }
    if (level == ISOMERIC_SUBSPECIES && sp_exception){
        headgoup_string << "(1)";
    }
        
    return headgoup_string.str();
}
        
        
ElementTable* Headgroup::get_elements(){
    if (use_headgroup){
        throw RuntimeException("Element table cannot be computed for lipid '" + headgroup + "'");
    }
    
    ElementTable *elements = create_empty_table();
    
    for (auto &kv : LipidClasses::get_instance().lipid_classes.at(lipid_class).elements){
        elements->at(kv.first) += kv.second;
    }
    
    for (auto hgd : *decorators){
        for (auto &kv : *(hgd->elements)){
            elements->at(kv.first) += kv.second * hgd->count;
        }
    }
    
    
    if (lipid_category == SP && (LipidClasses::get_instance().lipid_classes.at(lipid_class).class_name.compare("Cer") == 0 || LipidClasses::get_instance().lipid_classes.at(lipid_class).class_name.compare("SPB") == 0) && decorators->size() == 0){
        elements->at(ELEMENT_O) -= 1;
    }
    
    return elements;
}
