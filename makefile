.SUFIXES:
.SUFIXES: .c .o .h


CC				 = 	clang $(FLAGS)
FLAGS			 = 	--debug -g
BIN 			 = 	main
OBJ 			 = 	main.o http.o helpers.o
FILES 			 = 	*.c *.h

$(BIN):				$(OBJ)
	$(CC) -o 		$(BIN) $(OBJ)

main.o:    			main.c
	$(CC) -c -o 	main.o main.c

http.o:    			http.c http.h
	$(CC) -c -o 	http.o http.c

helpers.o:    		helpers.c helpers.h
	$(CC) -c -o 	helpers.o helpers.c

# $(OBJ): 			$(FILES)

run: 				$(BIN)
	./$(BIN)

clean:
	rm 				*.o main

.PHONY: run clean
