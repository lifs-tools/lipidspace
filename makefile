CC = g++
obj = Adduct.o IsomericFattyAcid.o LipidMolecularSubspecies.o LipidStructuralSubspecies.o FattyAcid.o  LipidAdduct.o LipidSpecies.o MolecularFattyAcid.o Fragment.o LipidIsomericSubspecies.o LipidSpeciesInfo.o StructuralFattyAcid.o
opt = -O0

main: ${obj}
	${CC} -shared ${obj} ${opt} -o libcppGoslin.so
	
	
%.o: cppgoslin/domain/%.cpp
	${CC} -std=c++11 -I. -Wall ${opt} -fPIC -c $<
