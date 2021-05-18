OUTPUT_NAME = main

build:
	g++ -lncurses -lmenu main.cpp -o $(OUTPUT_NAME)

play:
	g++ -lncurses -lmenu main.cpp -o $(OUTPUT_NAME)
	./$(OUTPUT_NAME)

clean:
	rm $(OUTPUT_NAME)
