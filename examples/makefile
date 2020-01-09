CC = g++
opt = -O3 -std=c++11 -Wall
inc = /usr/include
lib_dir = /usr/lib
libs = cppGoslin

main: lipid_name_parser.cpp simple_statistics.cpp
	${CC} ${opt} -I ${inc} -o lipid_name_parser lipid_name_parser.cpp -L ${lib_dir} -l ${libs}
	${CC} ${opt} -I ${inc} -o simple_statistics simple_statistics.cpp -L ${lib_dir} -l ${libs}
