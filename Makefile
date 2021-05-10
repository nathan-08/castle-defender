INCLUDE := -I/Library/Frameworks/SDL2.framework/Headers -I/Library/Frameworks/SDL2_ttf.framework/Headers -I/Library/Frameworks/SDL2_mixer.framework/Headers -I include
FW := -framework SDL2 -framework SDL2_ttf -framework SDL2_mixer -F/Library/Frameworks
COMPILER := g++ -std=c++17 -Wall

main: main.o app.o init.o agent.o textManager.o tileMatrix.o graph.o
	$(COMPILER) -o main main.o app.o init.o agent.o textManager.o  tileMatrix.o graph.o $(FW) 

main.o: main.cpp
	$(COMPILER) -c $< -I include

app.o: app.cpp app.hpp direction.hpp TextManager.hpp agent.hpp Graph.hpp TileMatrix.hpp
	$(COMPILER) -c $< $(INCLUDE)

init.o: init.cpp init.hpp const.hpp
	$(COMPILER) -c $< $(INCLUDE)

agent.o: agent.cpp agent.hpp direction.hpp
	$(COMPILER) -c $< $(INCLUDE)

textManager.o: TextManager.cpp TextManager.hpp
	$(COMPILER) -c $< $(INCLUDE)

tileMatrix.o: TileMatrix.cpp TileMatrix.hpp
	$(COMPILER) -c $< $(INCLUDE)

graph.o: graph.cpp Graph.hpp priorityQueue.hpp TileMatrix.hpp
	$(COMPILER) -c $< $(INCLUDE)

vpath %.cpp src
vpath %.hpp include

