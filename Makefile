TARGET = redis-cli

redis:
	gcc main.c server.c parser.c structure.c instructions.c file.c hash.c $(DEF) -o $(TARGET)
valgrind:
	valgrind --leak-check=full ./$(TARGET)
clean:
	rm $(TARGET)
