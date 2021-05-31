#include "cppgoslin/domain/HeadGroup.h"
    
Headgroup::Headgroup(string _headgroup, vector<string>* _decorators, bool _use_headgroup){
    self.headgroup = headgroup
    lipid_category = (_lipid_category != NO_CATEGORY) ? _lipid_category : get_category(headgroup);
    lipid_class = (_lipid_class != NO_CLASS) ? _lipid_class : get_class(headgroup);
    use_headgroup = _use_headgroup
    decorators = _decorators;
    sp_exception = lipid_category == SP && ((LipidClasses::get_instance().lipid_classes.at(lipid_class).class_name.compare("Cer") != 0 && LipidClasses::get_instance().lipid_classes.at(lipid_class).class_name.compare("SPB")) || decorators.size > 0);
}
        
        
    def get_lipid_string(self, level = None):
        if level == LipidLevel.CATEGORY:
            return self.lipid_category.name
        
        elif level == LipidLevel.CLASS:
            return all_lipids[self.lipid_class]["name"] if not self.use_headgroup else self.headgroup
        
        headgoup_string = [all_lipids[self.lipid_class]["name"] if not self.use_headgroup else self.headgroup]
                
        if level not in {LipidLevel.ISOMERIC_SUBSPECIES, LipidLevel.STRUCTURAL_SUBSPECIES}:
            prefix = sorted(hgd.to_string(level) for hgd in self.decorators if not hgd.suffix)
        else:
            prefix = ["%s-" % hgd.to_string(level) for hgd in self.decorators if not hgd.suffix]
            
        suffix = [hgd.to_string(level) for hgd in self.decorators if hgd.suffix]
        if level == LipidLevel.ISOMERIC_SUBSPECIES and self.sp_exception:
            suffix.append("(1)")
            
        return "".join(prefix + headgoup_string + suffix)
        
        
        
    def get_elements(self):
        if self.use_headgroup:
            raise LipidException("Element table cannot be computed for lipid level '%s'" % self.info.level)
        
        dummy = FunctionalGroup("dummy", elements = all_lipids[self.lipid_class]["elements"])
        for hgd in self.decorators: dummy += hgd
        
        if self.lipid_category == LipidCategory.SP and all_lipids[self.lipid_class]["name"] in {"Cer", "SPB"} and len(self.decorators) == 0:
            dummy.elements[Element.O] -= 1
        
        return dummy.elements
