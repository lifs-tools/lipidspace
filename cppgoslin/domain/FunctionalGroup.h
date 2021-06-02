#ifndef FUNCTIONALGROUP_H
#define FUNCTIONALGROUP_H

#include "cppgoslin/domain/Element.h"
#include "cppgoslin/domain/LipidEnums.h"
#include "cppgoslin/domain/DoubleBonds.h"
#include <map>
#include <vector>
#include <string>

using namespace std;
using namespace goslin;


#define contains(X, Y) ((X).find(Y) != (X).end())
#define contains_p(X, Y) ((X)->find(Y) != (X)->end())
#define uncontains(X, Y) ((X).find(Y) == (X).end())
#define uncontains_p(X, Y) ((X)->find(Y) == (X)->end())

class FunctionalGroup {
public:
    string name;
    int position;
    int count;
    string stereochemistry;
    string ringStereo;
    DoubleBonds* doubleBonds;
    bool is_atomic;
    ElementTable* elements;
    map<string, vector<FunctionalGroup*>>* functionalGroups;
    
    
    
    FunctionalGroup(string _name, int _position = -1, int _count = 1, DoubleBonds* _doubleBonds = 0, bool _is_atomic = false, string _stereochemistry = "", ElementTable* _elements = 0, map<string, vector<FunctionalGroup*>>* _functionalGroups = 0);
    FunctionalGroup(FunctionalGroup* fg);
    ~FunctionalGroup();
    ElementTable* getElements();
    void shiftPositions(int shift);
    ElementTable* getFunctionalGroupElements();
    void computeElements();
    string toString(LipidLevel level);
    int getDoubleBonds();
    void add(FunctionalGroup* fg);
};


class HeadgroupDecorator : public FunctionalGroup {
public:
    string suffix;
    LipidLevel lowestVisibleLevel;
    
    HeadgroupDecorator(string _name, int _position = -1, int _count = 1, ElementTable* _elements = 0, bool _suffix = false, LipidLevel _level = NO_LEVEL); 
    HeadgroupDecorator(HeadgroupDecorator* hgd);
    string toString(LipidLevel level);
};


class KnownFunctionalGroups {
    public:
        static KnownFunctionalGroups& get_instance()
        {
            static KnownFunctionalGroups instance;
            return instance;
        }
    private:
        KnownFunctionalGroups();
        
    public:
        map<string, FunctionalGroup*> knownFunctionalGroups;
        KnownFunctionalGroups(KnownFunctionalGroups const&) = delete;
        void operator=(KnownFunctionalGroups const&) = delete;
};
    
    
/*
    
_known_functionalGroups = {"OH": FunctionalGroup("OH", elements = {Element.O: 1}), # hydroxyl
                           "Me": FunctionalGroup("Me", elements = {Element.C: 1, Element.H: 2}), # methyl
                           "My": FunctionalGroup("My", elements = {Element.C: 1}, double_bonds = 1), # methylene
                           "dMe": FunctionalGroup("dMe", elements = {Element.C: 1}), # methylen
                           "oxo": FunctionalGroup("oxo", elements = {Element.O: 1, Element.H: -2}, double_bonds = 1), # keto
                           "COOH": FunctionalGroup("COOH", elements = {Element.C: 1, Element.O: 2}, double_bonds = 1), # carboxyl
                           "Ep": FunctionalGroup("Ep", elements = {Element.O: 1, Element.H: -2}, double_bonds = 1), # epoxy
                           "OO": FunctionalGroup("OO", elements = {Element.O: 2}),  # peroxy
                           "OMe": FunctionalGroup("OMe", elements = {Element.O: 1, Element.C: 1, Element.H: 2}), # methoxy
                           "oxy": FunctionalGroup("oxy", elements = {Element.O: 1}), # Alkoxy / ether
                           "Et": FunctionalGroup("Et", elements = {Element.C: 2, Element.H: 4}), # ethyl
                           "NH2": FunctionalGroup("NH2", elements = {Element.N: 1, Element.H: 1}),
                           "NO2": FunctionalGroup("NO2", elements = {Element.N: 1, Element.O: 2, Element.H: -1}, double_bonds = 1),
                           "OOH": FunctionalGroup("OOH", elements = {Element.O: 2}),
                           "SH": FunctionalGroup("SH", elements = {Element.S: 1}),
                           "CN": FunctionalGroup("CN", elements = {Element.C: 1, Element.N: 1, Element.H: -1}),
                           "Ph": FunctionalGroup("Ph", elements = {Element.P: 1, Element.O: 4, Element.H: 1}),
                           "Su": FunctionalGroup("Su", elements = {Element.S: 1, Element.O: 4}),
                           "T": FunctionalGroup("T", elements = {Element.S: 1, Element.O: 3, Element.H: 1}),
                           "G": FunctionalGroup("G", elements = {Element.N: 1, Element.H: 1}),
                           "Hex": HeadgroupDecorator("Hex", elements = {Element.C: 6, Element.H: 10, Element.O: 5}),
                           "Gal": HeadgroupDecorator("Gal", elements = {Element.C: 6, Element.H: 10, Element.O: 5}),
                           "Glc": HeadgroupDecorator("Glc", elements = {Element.C: 6, Element.H: 10, Element.O: 5}),
                           "NeuAc": HeadgroupDecorator("NeuAc", elements = {Element.O: 8, Element.N: 1, Element.C: 11, Element.H: 17}),
                           "SGal": HeadgroupDecorator("SGal", elements = {Element.O: 8, Element.H: 10, Element.C: 6, Element.S: 1}),
                           "S(3')Gal": HeadgroupDecorator("S(3')Gal", elements = {Element.O: 8, Element.H: 10, Element.C: 6, Element.S: 1}),
                           "S(3′)Gal": HeadgroupDecorator("S(3')Gal", elements = {Element.O: 8, Element.H: 10, Element.C: 6, Element.S: 1}),
                           "SHex": HeadgroupDecorator("SHex", elements = {Element.O: 8, Element.H: 10, Element.C: 6, Element.S: 1}),
                           "S(3')Hex": HeadgroupDecorator("S(3')Hex", elements = {Element.O: 8, Element.H: 10, Element.C: 6, Element.S: 1}),
                           "S(3′)Hex": HeadgroupDecorator("S(3')Hex", elements = {Element.O: 8, Element.H: 10, Element.C: 6, Element.S: 1}),
                           "GlcA": HeadgroupDecorator("GlcA", elements = {Element.C: 6, Element.O: 6, Element.H: 8}),
                           "HexA": HeadgroupDecorator("HexA", elements = {Element.C: 6, Element.O: 6, Element.H: 8}),
                           "HexNAc": HeadgroupDecorator("NexNAc", elements = {Element.C: 8, Element.H: 13, Element.O: 5, Element.N: 1}),
                           "GalNAc": HeadgroupDecorator("GalNAc", elements = {Element.C: 8, Element.H: 13, Element.O: 5, Element.N: 1}),
                           "GlcNAc": HeadgroupDecorator("GlcNAc", elements = {Element.C: 8, Element.H: 13, Element.O: 5, Element.N: 1}),
                           "Man": HeadgroupDecorator("Man", elements = {Element.C: 6, Element.H: 9, Element.O: 5}),
                           "Neu": HeadgroupDecorator("Neu", elements = {Element.C: 9, Element.H: 14, Element.O: 7, Element.N: 1}),
                           "NeuGc": HeadgroupDecorator("NeuGc", elements = {Element.C: 11, Element.H: 17, Element.N: 1, Element.O: 9}),
                           "NAc": HeadgroupDecorator("NAc", elements = {Element.C: 5, Element.H: 7, Element.N: 1, Element.O: 2, Element.S: 1}),
                           "Nac": HeadgroupDecorator("Nac", elements = {Element.C: 5, Element.H: 7, Element.N: 1, Element.O: 2, Element.S: 1}),
                           "Fuc": HeadgroupDecorator("Fuc", elements = {Element.C: 6, Element.H: 10, Element.O: 4}),
                           "Kdn": HeadgroupDecorator("Kdn", elements = {Element.C: 9, Element.H: 14, Element.O: 8}),
                           "Xyl": HeadgroupDecorator("Xyl", elements = {Element.C: 29, Element.H: 52, Element.O: 26}),
                           "COG": HeadgroupDecorator("COG", elements = {Element.C: 18, Element.H: 19, Element.N: 5, Element.O: 1}),
                           "COT": HeadgroupDecorator("COT", elements = {Element.C: 6, Element.H: 14, Element.N: 2, Element.O: 2}),
                           "OGlc": HeadgroupDecorator("OGlc", elements = {Element.C: 6, Element.H: 10, Element.O: 5}),
                           "H": FunctionalGroup("H", elements = {Element.H: 1}),
                           
                           "OGlcNAc": HeadgroupDecorator("OGlcNAc", elements = {}),
                           "NeuAc2": HeadgroupDecorator("NeuAc2", elements = {}),
                           
                           "O": FunctionalGroup("O", elements = {Element.O: 1}, is_atomic = True),
                           "N": FunctionalGroup("N", elements = {Element.N: 1, Element.H: 1}, is_atomic = True),
                           "P": FunctionalGroup("P", elements = {Element.P: 1, Element.H: 1}, is_atomic = True),
                           "S": FunctionalGroup("S", elements = {Element.S: 1}, is_atomic = True),
                           "Br": FunctionalGroup("Br", elements = {Element.Br: 1, Element.H: -1}, is_atomic = True),
                           "F": FunctionalGroup("F", elements = {Element.F: 1, Element.H: -1}, is_atomic = True),
                           "I": FunctionalGroup("I", elements = {Element.I: 1, Element.H: -1}, is_atomic = True),
                           "Cl": FunctionalGroup("Cl", elements = {Element.Cl: 1, Element.H: -1}, is_atomic = True),
                           "As": FunctionalGroup("As", elements = {Element.As: 1, Element.H: 1}, is_atomic = True),
                           
                           "Ac": FunctionalGroup("Ac", elements = {Element.C: 2, Element.O: 2, Element.H: 2}, double_bonds = 1),
                           "Cys": FunctionalGroup("Cys", elements = {Element.S: 1, Element.C: 3, Element.N: 1, Element.O: 2, Element.H: 5}, double_bonds = 1),
                           "Phe": FunctionalGroup("Phe", elements = {Element.C: 6, Element.H: 4}, double_bonds = 4),
                           "SGlu": FunctionalGroup("SGlu", elements = {Element.C: 10, Element.H: 15, Element.N: 3, Element.O: 6, Element.S: 1}, double_bonds = 4),
                           "SCys": FunctionalGroup("SCys", elements = {Element.C: 3, Element.H: 5, Element.N: 1, Element.O: 2, Element.S: 1}, double_bonds = 1),
                           "BOO": FunctionalGroup("BOO", elements = {Element.C: 4, Element.H: 8, Element.O: 2}),
                           "MMAs": FunctionalGroup("MMAs", elements = {Element.C: 2, Element.H: 5, Element.O: 1, Element.As: 1}),
                           "SMe": FunctionalGroup("SMe", elements = {Element.C: 1, Element.H: 2, Element.S: 1}),
                           "NH": FunctionalGroup("NH", elements = {Element.N: 1, Element.H: -1}, double_bonds = 1),
                           "SCG": FunctionalGroup("SCG", elements = {Element.C: 5, Element.H: 8, Element.O: 3, Element.N: 2, Element.S: 1}, double_bonds = 2),
                           }



def get_functional_group(name):
    if name in _known_functionalGroups:
        return _known_functionalGroups[name].copy()
    raise Exception("Name '%s' not registered in functional group list" % name)

*/

#endif /* FUNCTIONALGROUP_H */
        
        
