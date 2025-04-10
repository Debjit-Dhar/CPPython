TARGET = a.out
CXX = c++
FLAGS = 

#Header files list here. Change whenever new header is added.
HEADERS = lexer.hpp parser.hpp symtab.hpp

all: $(TARGET)

lexer.o: lexer.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

parser.o: parser.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

grammar.o: grammar.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

firstfollow.o: firstfollow.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

item.o: item.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

tablegen.o: tablegen.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

main.o: main.cpp $(HEADERS)
	$(CXX) $(FLAGS) -c $< -o $@

$(TARGET): lexer.o parser.o grammar.o firstfollow.o item.o tablegen.o main.o
	$(CXX) $(FLAGS) $^ -o $@

clean:
	rm -f *.o

all-clean: clean
	rm -f $(TARGET)