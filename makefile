install_dir = /usr
CC = g++ -std=c++11
AR = ar
MARCH = -mtune=native
bin = libcppGoslin.so
abin = libcppGoslin.a
domain = cppgoslin/domain/Adduct.o cppgoslin/domain/LipidMolecularSubspecies.o cppgoslin/domain/LipidStructuralSubspecies.o cppgoslin/domain/FattyAcid.o cppgoslin/domain/LipidAdduct.o cppgoslin/domain/LipidSpecies.o cppgoslin/domain/Fragment.o cppgoslin/domain/LipidIsomericSubspecies.o cppgoslin/domain/LipidSpeciesInfo.o cppgoslin/domain/StringFunctions.o

parser = cppgoslin/parser/ParserClasses.o cppgoslin/parser/KnownParsers.o cppgoslin/parser/GoslinFragmentParserEventHandler.o cppgoslin/parser/GoslinParserEventHandler.o cppgoslin/parser/LipidMapsParserEventHandler.o cppgoslin/parser/SwissLipidsParserEventHandler.o cppgoslin/parser/HmdbParserEventHandler.o cppgoslin/parser/SumFormulaParserEventHandler.o

obj = ${domain} ${parser}
test_obj = cppgoslin/tests/FattyAcidTest.o cppgoslin/tests/ParserTest.o cppgoslin/tests/SwissLipidsTest.o cppgoslin/tests/GoslinTest.o cppgoslin/tests/LipidMapsTest.o cppgoslin/tests/HdmbTest.o

opt = -O3 ${MARCH} -Wall -fstack-protector-strong -D_FORTIFY_SOURCE=2


main: ${bin}

${bin}:	cppgoslin/parser/KnownGrammars.h cppgoslin/domain/LipidEnums.h ${obj}
	${CC} -shared ${obj} -o ${bin}
	
	
static: cppgoslin/parser/KnownGrammars.h cppgoslin/domain/LipidEnums.h ${obj}
	${AR} rcs ${abin} ${obj}

	
cppgoslin/parser/KnownGrammars.h: data/goslin/Goslin.g4 data/goslin/GoslinFragments.g4 data/goslin/LipidMaps.g4 data/goslin/SwissLipids.g4
	${CC} ${opt} -I . -o writeGrammarsHeader writeGrammarsHeader.cpp && ./writeGrammarsHeader "cppgoslin/parser/KnownGrammars.h"
	
cppgoslin/domain/LipidEnums.h: data/goslin/lipid-list.csv
	${CC} ${opt} -I . -o writeLipidEnums writeLipidEnums.cpp cppgoslin/domain/StringFunctions.cpp cppgoslin/parser/SumFormulaParserEventHandler.cpp cppgoslin/parser/ParserClasses.cpp && ./writeLipidEnums "cppgoslin/domain/LipidEnums.h"
	

	
%.o: %.cpp cppgoslin/parser/KnownGrammars.h cppgoslin/domain/LipidEnums.h
	${CC} ${opt} -I. -Wall -fPIC -o $@ -c $<
	
clean:
	rm -f "cppgoslin/parser/KnownGrammars.h"
	rm -f "cppgoslin/domain/LipidEnums.h"
	rm -f cppgoslin/domain/*.o
	rm -f cppgoslin/parser/*.o
	rm -f cppgoslin/tests/*.o
	rm -f ${bin}
	rm -f *Test
	rm -f writeGrammarsHeader
	rm -f writeLipidEnums
	rm -f ${abin}
	
dist-clean: clean
	rm -f ${install_dir}/lib/${bin}
	rm -rf ${install_dir}/include/cppgoslin


install: ${bin}
	mkdir -p ${install_dir}/lib
	mkdir -p ${install_dir}/include
	mkdir -p ${install_dir}/include/cppgoslin
	mkdir -p ${install_dir}/include/cppgoslin/domain
	mkdir -p ${install_dir}/include/cppgoslin/parser
	cp ${bin} ${install_dir}/lib
	cp cppgoslin/cppgoslin.h ${install_dir}/include/cppgoslin/.
	cp cppgoslin/domain/*.h  ${install_dir}/include/cppgoslin/domain/.
	cp cppgoslin/parser/*.h  ${install_dir}/include/cppgoslin/parser/.
	
	
FattyAcidTest: cppgoslin/tests/FattyAcidTest.o libcppGoslin.so
	${CC} -I. ${opt} -Bstatic -o FattyAcidTest cppgoslin/tests/FattyAcidTest.o libcppGoslin.so

ParserTest: cppgoslin/tests/ParserTest.o libcppGoslin.so
	${CC} -I. ${opt} -Bstatic -o ParserTest cppgoslin/tests/ParserTest.o libcppGoslin.so
	
LipidMapsTest: cppgoslin/tests/LipidMapsTest.o libcppGoslin.so
	${CC} -I. ${opt} -Bstatic -o LipidMapsTest cppgoslin/tests/LipidMapsTest.o libcppGoslin.so
	
GoslinTest: cppgoslin/tests/GoslinTest.o libcppGoslin.so
	${CC} -I. ${opt} -Bstatic -o GoslinTest cppgoslin/tests/GoslinTest.o libcppGoslin.so
	
SwissLipidsTest: cppgoslin/tests/SwissLipidsTest.o libcppGoslin.so
	${CC} -I. ${opt} -Bstatic -o SwissLipidsTest cppgoslin/tests/SwissLipidsTest.o libcppGoslin.so
	
HmdbTest: cppgoslin/tests/HmdbTest.o libcppGoslin.so
	${CC} -I. ${opt} -Bstatic -o HmdbTest cppgoslin/tests/HmdbTest.o libcppGoslin.so
	
	
test: FattyAcidTest ParserTest LipidMapsTest GoslinTest SwissLipidsTest HmdbTest

	
runtests: FattyAcidTest ParserTest LipidMapsTest GoslinTest SwissLipidsTest HmdbTest
	LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH} ./FattyAcidTest
	LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH} ./ParserTest
	LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH} ./LipidMapsTest
	LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH} ./GoslinTest
	LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH} ./SwissLipidsTest
	LD_LIBRARY_PATH=.:${LD_LIBRARY_PATH} ./HmdbTest
