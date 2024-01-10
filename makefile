.SUFIXES:
.SUFIXES: .c .o .h


COMP			 =	$(CC) $(FLAGS)
FLAGS			 = 	--debug -g
BIN 			 = 	main
OBJ 			 = 	main.o http.o helpers.o
FILES 			 = 	*.c *.h

$(BIN):				$(OBJ)
	$(COMP) -o 		$(BIN) $(OBJ)

main.o:    			main.c
	$(COMP) -c -o 	main.o main.c

http.o:    			http.c http.h
	$(COMP) -c -o 	http.o http.c

helpers.o:    		helpers.c helpers.h
	$(COMP) -c -o 	helpers.o helpers.c

# $(OBJ): 			$(FILES)

run: 				$(BIN)
	./$(BIN)

clean:
	rm 				*.o main

.PHONY: run clean
