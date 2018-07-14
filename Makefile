all: test.exe

test.exe: main.o top.o  RRAM.o RRAM_Controller.o
	g++ -L$(SYSTEMC_HOME)/lib-linux64 main.o top.o RRAM.o RRAM_Controller.o  -lsystemc -o test.exe

RRAM_Controller.o: RRAM_Controller.cpp RRAM_Controller.h
	g++ -c -I$(SYSTEMC_HOME)/include RRAM_Controller.cpp

RRAM.o: RRAM.cpp RRAM.h
	g++ -c -I$(SYSTEMC_HOME)/include RRAM.cpp

top.o: top.cpp top.h
	g++ -c -I$(SYSTEMC_HOME)/include top.cpp

main.o: main.cpp top.h
	g++ -c -I$(SYSTEMC_HOME)/include main.cpp
run:
	./test.exe
clean: 
	rm  ./*.o ./test.exe ./*.vcd ./*.log*

