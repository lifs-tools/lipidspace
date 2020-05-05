#ifndef ELEMENT_H_FILE
#define ELEMENT_H_FILE

#define ELECTRON_REST_MASS 0.00054857990946

#include <string>
#include <map>
#include <vector>
#include <sstream>

namespace goslin {
using namespace std;

enum Element {ELEMENT_C, ELEMENT_C13, ELEMENT_H, ELEMENT_H2, ELEMENT_N, ELEMENT_N15, ELEMENT_O, ELEMENT_O17, ELEMENT_O18, ELEMENT_P, ELEMENT_P32, ELEMENT_S, ELEMENT_S34, ELEMENT_S33};


typedef map<Element, int> ElementTable;


ElementTable* create_empty_table();
string compute_sum_formula(ElementTable* elements);

const map<string, Element> element_positions = {{"C", ELEMENT_C}, {"H", ELEMENT_H}, {"N", ELEMENT_N}, {"O", ELEMENT_O}, {"P", ELEMENT_P}, {"P'", ELEMENT_P32}, {"S", ELEMENT_S}, {"S'", ELEMENT_S34}, {"S''", ELEMENT_S33}, {"H'", ELEMENT_H2}, {"C'", ELEMENT_C13}, {"N'", ELEMENT_N15}, {"O'", ELEMENT_O17}, {"O''", ELEMENT_O18}, {"2H", ELEMENT_H2}, {"13C", ELEMENT_C13}, {"15N", ELEMENT_N15}, {"17O", ELEMENT_O17}, {"18O", ELEMENT_O18}, {"32P", ELEMENT_P32}, {"34S", ELEMENT_S34}, {"33S", ELEMENT_S33}, {"H2", ELEMENT_H2}, {"C13", ELEMENT_C13}, {"N15", ELEMENT_N15}, {"O17", ELEMENT_O17}, {"O18", ELEMENT_O18}, {"P32", ELEMENT_P32}, {"S34", ELEMENT_S34}, {"S33", ELEMENT_S33}};


const map<Element, double> element_masses = {{ELEMENT_C, 12.0},  {ELEMENT_H, 1.007825035},  {ELEMENT_N, 14.0030740}, {ELEMENT_O, 15.99491463}, {ELEMENT_P, 30.973762},  {ELEMENT_S, 31.9720707}, {ELEMENT_H2, 2.014101779},  {ELEMENT_C13, 13.0033548378},  {ELEMENT_N15, 15.0001088984}, {ELEMENT_O17, 16.9991315}, {ELEMENT_O18, 17.9991604}, {ELEMENT_P32, 31.973907274}, {ELEMENT_S33, 32.97145876}, {ELEMENT_S34, 33.96786690}};



const map<Element, string> element_shortcut = {{ELEMENT_C, "C"}, {ELEMENT_H, "H"}, {ELEMENT_N, "N"}, {ELEMENT_O, "O"}, {ELEMENT_P, "P"}, {ELEMENT_S, "S"}, {ELEMENT_H2, "H'"}, {ELEMENT_C13, "C'"}, {ELEMENT_N15, "N'"}, {ELEMENT_O17, "O'"}, {ELEMENT_O18, "O''"}, {ELEMENT_P32, "P'"}, {ELEMENT_S33, "S'"}, {ELEMENT_S34, "S''"}};


    

const vector<Element> element_order = {ELEMENT_C, ELEMENT_H, ELEMENT_N, ELEMENT_O, ELEMENT_P, ELEMENT_S, ELEMENT_H2, ELEMENT_C13, ELEMENT_N15, ELEMENT_O17, ELEMENT_O18, ELEMENT_P32, ELEMENT_S33, ELEMENT_S34};
}



#endif /* ELEMENT_H_FILE */
