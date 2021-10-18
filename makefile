CC = g++ -std=c++11
MARCH = -mtune=native
bin_pre = precompute_classes
opt = -O3 ${MARCH} -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fopenmp
obj_pre = src/MCIS2NK.o src/TreeGen.o src/auxiliary.o src/bbpmcsi.o src/graphVisualization.o src/hungarian.o src/main_precompute.o src/mwm.o

bin = lipidspace
obj = src/lipidspace.o

main: ${bin_pre} ${bin}

${bin}: ${obj}
	${CC} ${opt} ${obj} -o ${bin} -lpython3.8 -lcppGoslin

${bin_pre}: ${obj_pre}
	${CC} ${opt} ${obj_pre} -o ${bin_pre} -L ~/workspace/lib -lOGDF -lCOIN
	
%.o: %.cpp
	${CC} ${opt} -I /usr/local/include/eigen3 -I /usr/include/python3.8 -I . -I ~/workspace/include -o $@ -c $<

clean:
	rm -f ${obj_pre}
	rm -f ${bin_pre}

yeast:
	./lipidspace output Yeast/BY4741_24 Yeast/BY4741_37 Yeast/Elo1_24 Yeast/Elo1_37 Yeast/Elo2_24 Yeast/Elo2_37 Yeast/Elo3_24 Yeast/Elo3_37

test:
	./lipidspace output examples/GP.csv examples/SP.csv
