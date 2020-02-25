install_dir = /usr
CC = g++
bin = libcppGoslin.so
domain = cppgoslin/domain/Adduct.o cppgoslin/domain/IsomericFattyAcid.o cppgoslin/domain/LipidMolecularSubspecies.o cppgoslin/domain/LipidStructuralSubspecies.o cppgoslin/domain/FattyAcid.o cppgoslin/domain/LipidAdduct.o cppgoslin/domain/LipidSpecies.o cppgoslin/domain/MolecularFattyAcid.o cppgoslin/domain/Fragment.o cppgoslin/domain/LipidIsomericSubspecies.o cppgoslin/domain/LipidSpeciesInfo.o cppgoslin/domain/StructuralFattyAcid.o

parser = cppgoslin/parser/ParserClasses.o cppgoslin/parser/KnownParsers.o cppgoslin/parser/GoslinFragmentParserEventHandler.o cppgoslin/parser/GoslinParserEventHandler.o cppgoslin/parser/LipidMapsParserEventHandler.o cppgoslin/parser/SwissLipidsParserEventHandler.o

obj = ${domain} ${parser}
test_obj = cppgoslin/tests/MolecularFattyAcidTest.o cppgoslin/tests/ParserTest.o cppgoslin/tests/SwissLipidsTest.o

opt = -std=c++11 -O3


main: grammarWriter ${obj}
	${CC} -shared ${obj} -o ${bin}
	
	

grammarWriter:
	${CC} ${opt} -o writeGrammarsHeader writeGrammarsHeader.cpp
	./writeGrammarsHeader "cppgoslin/parser/KnownGrammars.h"
	
	
%.o: %.cpp
	${CC} ${opt} -I. -Wall -fPIC -o $@ -c $<
	
clean:
	rm -f "cppgoslin/parser/KnownGrammars.h"
	rm -f cppgoslin/domain/*.o
	rm -f cppgoslin/parser/*.o
	rm -f cppgoslin/tests/*.o
	rm -f ${bin}
	rm -f MolecularFattyAcidTest
	rm -f ParserTest
	rm -f writeGrammarsHeader
	
dist-clean: clean
	rm -f ${install_dir}/lib/${bin}
	rm -rf ${install_dir}/include/cppgoslin


install: main
	mkdir -p ${install_dir}/lib
	mkdir -p ${install_dir}/include
	mkdir -p ${install_dir}/include/cppgoslin
	mkdir -p ${install_dir}/include/cppgoslin/domain
	mkdir -p ${install_dir}/include/cppgoslin/parser
	cp ${bin} ${install_dir}/lib
	cp cppgoslin/cppgoslin.h ${install_dir}/include/cppgoslin/.
	cp cppgoslin/domain/*.h  ${install_dir}/include/cppgoslin/domain/.
	cp cppgoslin/parser/*.h  ${install_dir}/include/cppgoslin/parser/.
	
test: libcppGoslin.so ${test_obj}
	${CC} -I. ${opt} -o MolecularFattyAcidTest cppgoslin/tests/MolecularFattyAcidTest.o -L. -l cppGoslin
	${CC} -I. ${opt} -o ParserTest cppgoslin/tests/ParserTest.o -L. -l cppGoslin
	${CC} -I. ${opt} -o SwissLipidsTest cppgoslin/tests/SwissLipidsTest.o -L. -l cppGoslin

	
runtests:
	./MolecularFattyAcidTest
	./ParserTest
	./SwissLipidsTest
