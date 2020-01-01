CC = g++
domain = cppgoslin/domain/Adduct.o cppgoslin/domain/IsomericFattyAcid.o cppgoslin/domain/LipidMolecularSubspecies.o cppgoslin/domain/LipidStructuralSubspecies.o cppgoslin/domain/FattyAcid.o cppgoslin/domain/LipidAdduct.o cppgoslin/domain/LipidSpecies.o cppgoslin/domain/MolecularFattyAcid.o cppgoslin/domain/Fragment.o cppgoslin/domain/LipidIsomericSubspecies.o cppgoslin/domain/LipidSpeciesInfo.o cppgoslin/domain/StructuralFattyAcid.o

parser = cppgoslin/parser/ParserClasses.o cppgoslin/parser/KnownParsers.o cppgoslin/parser/GoslinFragmentParserEventHandler.o cppgoslin/parser/GoslinParserEventHandler.o cppgoslin/parser/LipidMapsParserEventHandler.o

obj = ${domain} ${parser}

opt = -O0


main: ${obj}
	${CC} -shared ${obj} -o libcppGoslin.so
	
	
%.o: %.cpp
	${CC} -std=c++11 -I. -Wall ${opt} -fPIC -o $@ -c $<
	
clean:
	rm -rf cppgoslin/domain/*.o
	rm -rf cppgoslin/parser/*.o
	rm -rf libcppGoslin.so

test: main
	${CC} -I. -O3 -o MolecularFattyAcidTest cppgoslin/tests/MolecularFattyAcidTest.cpp -L. -l cppGoslin
	${CC} -I. -O3 -o ParserTest cppgoslin/tests/ParserTest.cpp -L. -l cppGoslin
