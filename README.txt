In order to run this repository follow these steps:
1. Clone the repository git clone "https://github.com/Debjit-Dhar/CPPython"
2. Execute the command- make
3. The files will be executed in the order-
c++  -c lexer.cpp -o lexer.o
c++  -c parser.cpp -o parser.o
c++  -c grammar.cpp -o grammar.o
c++  -c firstfollow.cpp -o firstfollow.o
c++  -c item.cpp -o item.o
c++  -c tablegen.cpp -o tablegen.o
c++  -c main.cpp -o main.o
c++  lexer.o parser.o grammar.o firstfollow.o item.o tablegen.o main.o -o a.out




THIS WORK WAS DONE BY DEBJIT DHAR, SOHAM LAHIRI, SUDIPTA SAHA AND CHIRANTAN NATH