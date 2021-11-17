CC = g++
MARCH = -mtune=native
bin = ClassMatrixComputation
opt = -fopenmp -std=c++11 -O3 ${MARCH} -fstack-protector-strong -D_FORTIFY_SOURCE=2
obj = src/MCIS2NK.o src/TreeGen.o src/auxiliary.o src/bbpmcsi.o src/graphVisualization.o src/hungarian.o src/main_precompute.o src/mwm.o


main: ${bin}

${bin}: ${obj}
	${CC} ${opt} ${obj} -o ${bin} -L ~/workspace/lib -lOGDF -lCOIN
	
%.o: %.cpp
	${CC} -fopenmp ${opt} -I ~/workspace/include -o $@ -c $<

clean:
	rm -f ${obj}
	rm -f ${bin}
