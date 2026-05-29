TARGET = redis-server
TARGET_TEST = server-test

stable:
	gcc -g src/main.c src/server.c src/parser.c src/structure.c src/instructions.c src/file.c src/hash.c src/utils.c -o $(TARGET) $(DEF) 
test:
	gcc -g src/main.c src/server.c src/parser.c src/structure.c src/instructions.c src/file.c src/hash.c src/utils.c- o $(TARGET_TEST) -DTEST $(DEF)
vg:
	valgrind --leak-check=full --track-origins=yes --max-stackframe=5243152 ./$(TARGET)
vgtest:
	valgrind --leak-check=full --track-origins=yes --max-stackframe=5243152 ./$(TARGET_TEST)
clean:
	rm $(TARGET) $(TARGET_TEST)
