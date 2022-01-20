MineSat: clean File.o Proof.o Solver.o MineSat.o
	g++ -o $@ -std=c++11 -g File.o Proof.o Solver.o MineSat.o

File.o: File.cpp
	g++ -c -std=c++11 -g File.cpp

Proof.o: Proof.cpp
	g++ -c -std=c++11 -g Proof.cpp

Solve.o: Solver.cpp
	g++ -c -std=c++11 -g Solver.cpp

MineSat.o: MineSat.cpp
	g++ -c -std=c++11 -g MineSat.cpp

clean:
	rm -f *.o satTest tags
