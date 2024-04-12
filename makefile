all:
	g++ -o test.exe -I./include -I./src src/main.cpp  -lglfw3 -lopengl32 -lglew32 -lglu32 -lkernel32 -lgdi32 -luser32 -lshell32 -lmingw32 -std=c++17