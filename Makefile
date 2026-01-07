TARGET = redis-cli

redis:
	gcc main.c parser.c ds.c hash.c fileh.c instructions.c $(DEF) -o $(TARGET)
valgrind:
	valgrind --leak-check=full ./$(TARGET)
clean:
	rm $(TARGET)
