.SUFIXES:
.SUFIXES: .c .o .h


COMP			 =	$(CC) $(CFLAGS)
CFLAGS			 = 	
BIN 			 = 	server
OBJ 			 = 	main.o helpers.o http_server.o http_request.o http_response.o
FILES 			 = 	*.c *.h

$(BIN):				$(OBJ) $(FILES)
	$(COMP) -o  	$(BIN) $(OBJ)

main.o:    			main.c $(FILES)
	$(COMP) -c -o 	main.o main.c

helpers.o: 			helpers.c helpers.h
	$(COMP) -c -o 	helpers.o helpers.c

http_server.o: 		http_server.c http_server.h
	$(COMP) -c -o 	http_server.o http_server.c

http_request.o: 	http_request.c
	$(COMP) -c -o 	http_request.o http_request.c

http_response.o: 	http_response.c
	$(COMP) -c -o 	http_response.o http_response.c

debug: CFLAGS += -g --debug
debug: $(BIN)

test: CFLAGS += -fsanitize=address
test: debug


run: 				$(BIN)
	./$(BIN)

clean:
	rm 				*.o $(BIN)

.PHONY: run clean debug
