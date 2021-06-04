#include "cppgoslin/domain/Headgroup.h"
    
Headgroup::Headgroup(string _headgroup, vector<HeadgroupDecorator*>* _decorators, bool _useHeadgroup){
    headgroup = _headgroup;
    lipidCategory = getCategory(_headgroup);
    lipidClass = getClass(headgroup);
    useHeadgroup = _useHeadgroup;
    decorators = (_decorators != 0) ? _decorators : new vector<HeadgroupDecorator*>();
    spException = lipidCategory == SP && ((LipidClasses::get_instance().lipid_classes.at(lipidClass).class_name.compare("Cer") != 0 && LipidClasses::get_instance().lipid_classes.at(lipidClass).class_name.compare("SPB") != 0) || decorators->size() > 0);
}


Headgroup::~Headgroup(){
    for (auto hgd : *decorators) delete hgd;
    delete decorators;
}
        

LipidCategory Headgroup::getCategory(string _headgroup){
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


LipidLevel Headgroup::getLipidLevel(){
    return SPECIES;
}


LipidClass Headgroup::getClass(string _headgroup){
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


string Headgroup::getClassString(LipidClass _lipidClass){
    if (!ClassString.size()){
        for (auto kvp : LipidClasses::get_instance().lipid_classes){
            ClassString.insert({kvp.first, kvp.second.synonyms.at(0)});
        }
    }
    auto cl = ClassString.find(_lipidClass);
    return (cl != ClassString.end()) ? ClassString.at(_lipidClass) : "UNDEFINED";
}


string Headgroup::getClassName(){
    return LipidClasses::get_instance().lipid_classes.at(lipidClass).class_name;
}


string Headgroup::getCategoryString(LipidCategory _lipidCategory){
    return CategoryString.at(_lipidCategory);
}        
        
        
string Headgroup::getLipidString(LipidLevel level){
    if (level == CATEGORY){
        return getCategoryString(lipidCategory);
    }
    
    string hgs = ((useHeadgroup) ? headgroup : getClassString(lipidClass));
    
    if (level == CLASS){
        return hgs;
    }
    
    stringstream headgoupString;
            
    // adding prefixes to the headgroup
    if (level != ISOMERIC_SUBSPECIES && level != STRUCTURAL_SUBSPECIES){
        vector<string> prefixes;
        for (auto hgd : *decorators){
            if (!hgd->suffix) prefixes.push_back(hgd->toString(level));
        }
        sort (prefixes.begin(), prefixes.end());
        for (auto prefix : prefixes) headgoupString << prefix;
    }
    else {
        for (auto hgd : *decorators){
            if (!hgd->suffix) headgoupString << hgd->toString(level) << "-";
        }
    }
        
    // adding headgroup
    headgoupString << hgs;
        
    // ading suffixes to the headgroup
    for (auto hgd : *decorators){
        if (hgd->suffix) headgoupString << hgd->toString(level);
    }
    if (level == ISOMERIC_SUBSPECIES && spException){
        headgoupString << "(1)";
    }
        
    return headgoupString.str();
}
        
        
ElementTable* Headgroup::getElements(){
    if (useHeadgroup){
        throw RuntimeException("Element table cannot be computed for lipid '" + headgroup + "'");
    }
    
    ElementTable *elements = create_empty_table();
    
    for (auto &kv : LipidClasses::get_instance().lipid_classes.at(lipidClass).elements){
        elements->at(kv.first) += kv.second;
    }
    
    for (auto hgd : *decorators){
        for (auto &kv : *(hgd->elements)){
            elements->at(kv.first) += kv.second * hgd->count;
        }
    }
    
    
    if (lipidCategory == SP && (LipidClasses::get_instance().lipid_classes.at(lipidClass).class_name.compare("Cer") == 0 || LipidClasses::get_instance().lipid_classes.at(lipidClass).class_name.compare("SPB") == 0) && decorators->size() == 0){
        elements->at(ELEMENT_O) -= 1;
    }
    
    return elements;
}
