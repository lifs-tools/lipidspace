CC = g++
obj = cppgoslin/domain/Adduct.o cppgoslin/domain/IsomericFattyAcid.o cppgoslin/domain/LipidMolecularSubspecies.o cppgoslin/domain/LipidStructuralSubspecies.o cppgoslin/domain/FattyAcid.o cppgoslin/domain/LipidAdduct.o cppgoslin/domain/LipidSpecies.o cppgoslin/domain/MolecularFattyAcid.o cppgoslin/domain/Fragment.o cppgoslin/domain/LipidIsomericSubspecies.o cppgoslin/domain/LipidSpeciesInfo.o cppgoslin/domain/StructuralFattyAcid.o
opt = -O0


main: ${obj}
	${CC} -shared ${obj} -o libcppGoslin.so
	
	
%.o: %.cpp
	${CC} -std=c++11 -I. -Wall ${opt} -fPIC -o $@ -c $<
	
clean:
	rm -rf cppgoslin/domain/*.o
	rm -rf libcppGoslin.so

test: main
	${CC} -I. -o MolecularFattyAcidTest cppgoslin/tests/MolecularFattyAcidTest.cpp -L. -l cppGoslin
