.SUFIXES:
.SUFIXES: .c .o .h


COMP			 =	$(CC) $(FLAGS)
FLAGS			 = 	--debug -g
BIN 			 = 	server
OBJ 			 = 	main.o http_server.o
FILES 			 = 	*.c *.h

$(BIN):				$(OBJ) $(FILES)
	$(COMP) -o  	$(BIN) $(OBJ)

main.o:    			main.c $(FILES)
	$(COMP) -c -o 	main.o main.c

http_server.o: 		http_server.c http_server.h
	$(COMP) -c -o 	http_server.o http_server.c

# $(OBJ): 			$(FILES)

run: 				$(BIN)
	./$(BIN)

clean:
	rm 				*.o $(BIN)

.PHONY: run clean
