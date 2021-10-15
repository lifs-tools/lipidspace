CC = g++ -std=c++11
MARCH = -mtune=native
bin = lipidSpace
opt = -O3 ${MARCH} -fstack-protector-strong -D_FORTIFY_SOURCE=2 -fopenmp
obj = src/MCIS2NK.o src/TreeGen.o src/auxiliary.o src/bbpmcsi.o src/graphVisualization.o src/hungarian.o src/main.o src/mwm.o

main: ${bin}

${bin}: ${obj}
	${CC} ${opt} ${obj} -o ${bin} -L ~/workspace/lib -lOGDF -lCOIN
	
%.o: %.cpp
	${CC} ${opt} -I . -I ~/workspace/include -o $@ -c $<

clean:
	rm -f ${obj}
	rm -f ${bin}
