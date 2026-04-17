TARGET = redis-server
TARGET_TEST = server-test

redis:
	gcc -g src/main.c src/server.c src/parser.c src/structure.c src/instructions.c src/file.c src/hash.c $(DEF) -o $(TARGET)
test:
	gcc -g src/main.c src/server.c src/parser.c src/structure.c src/instructions.c src/file.c src/hash.c src/test.c src/utils.c -DTEST $(DEF) -o $(TARGET_TEST)
vg:
	valgrind --leak-check=full --track-origins=yes ./$(TARGET)
vgtest:
	valgrind --leak-check=full --track-origins=yes ./$(TARGET_TEST)
clean:
	rm $(TARGET)
