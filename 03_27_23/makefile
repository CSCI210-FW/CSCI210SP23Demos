all : main main2
main: main.cpp
	g++ -pedantic-errors -g main.cpp -lsqlite3 -o main
main2: main2.cpp
	g++ -pedantic-errors -g main2.cpp -lsqlite3 -o main2

clean :
	rm main
	rm main2