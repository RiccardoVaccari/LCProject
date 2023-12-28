CPPFLAGS="-I/opt/homebrew/opt/llvm/include"

.PHONY: clean all

all: kcomp

kcomp:    driver.o parser.o scanner.o kcomp.o
	clang++ -Wno-everything -o kcomp driver.o parser.o scanner.o kcomp.o `llvm-config --cxxflags --ldflags --libs --libfiles --system-libs`

kcomp.o:  kcomp.cpp driver.hpp
	clang++ -Wno-everything -c kcomp.cpp -I/opt/homebrew/opt/llvm@17/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
	
parser.o: parser.cpp
	clang++ -Wno-everything -c parser.cpp -I/opt/homebrew/opt/llvm@17/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
	
scanner.o: scanner.cpp parser.hpp
	clang++ -Wno-everything -c scanner.cpp -I/opt/homebrew/opt/llvm@17/include -std=c++17 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 
	
driver.o: driver.cpp parser.hpp driver.hpp
	clang++ -Wno-everything -c driver.cpp -I/opt/homebrew/opt/llvm/include -std=c++17 -fno-exceptions -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS 

parser.cpp, parser.hpp: parser.yy 
	bison -o parser.cpp parser.yy

scanner.cpp: scanner.ll
	flex -o scanner.cpp scanner.ll

clean:
	rm -f *~ driver.o scanner.o parser.o kcomp.o kcomp scanner.cpp parser.cpp parser.hpp
