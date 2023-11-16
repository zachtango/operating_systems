all: file

file: main.cpp types.hpp fileTable.hpp fileSystem.hpp disk.hpp bitmap.hpp
	g++ -std=c++11 -pthread main.cpp -o file

debug: main.cpp types.hpp fileTable.hpp fileSystem.hpp disk.hpp bitmap.hpp
	g++ -std=c++11 -pthread -g main.cpp -o file

clean:
	rm file
