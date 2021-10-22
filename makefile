CC = g++
MARCH = -mtune=native
bin_pre = precompute_classes
opt = -fopenmp -std=c++11 -O3 ${MARCH} -fstack-protector-strong -D_FORTIFY_SOURCE=2
obj_pre = src/MCIS2NK.o src/TreeGen.o src/auxiliary.o src/bbpmcsi.o src/graphVisualization.o src/hungarian.o src/main_precompute.o src/mwm.o

bin = lipidspace
obj = src/lipidspace.o

main: ${bin_pre} ${bin}

${bin}: ${obj}
	${CC} -fopenmp ${obj} -o ${bin} -lpython3.8 -lcppGoslin

${bin_pre}: ${obj_pre}
	${CC} ${opt} ${obj_pre} -o ${bin_pre} -L ~/workspace/lib -lOGDF -lCOIN
	
%.o: %.cpp
	${CC} -fopenmp ${opt} -I LipidSpace/Eigen -I LipidSpace/Spectra/include/Spectra -I /usr/include/python3.8 -I . -I ~/workspace/include -o $@ -c $<

clean:
	rm -f ${obj_pre}
	rm -f ${bin_pre}

yeast:
	./lipidspace output lists Yeast/BY4741_24 Yeast/BY4741_37 Yeast/Elo1_24 Yeast/Elo1_37 Yeast/Elo2_24 Yeast/Elo2_37 Yeast/Elo3_24 Yeast/Elo3_37
	
fly:
	./lipidspace output lists Drosophila/DmBraiPF  Drosophila/DmBraiYF  Drosophila/DmFaboPF  Drosophila/DmFaboYF  Drosophila/DmGutPF  Drosophila/DmGutYF  Drosophila/DmLipoPF  Drosophila/DmLipoYF  Drosophila/DmSaglPF  Drosophila/DmSaglYF  Drosophila/DmWidsPF  Drosophila/DmWidsYF

test:
	./lipidspace output lists examples/GP.csv examples/SP.csv
	
example:
	./lipidspace output lists examples/mouse-brain.csv examples/mouse-heart.csv examples/mouse-platelet.csv examples/human-plasma.csv examples/human-platelet.csv
