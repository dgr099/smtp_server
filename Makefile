CC = g++
CFLAGS = -g -O2
CFLAGS_EXTRA = -lpthread
BIN_FILE = smtp
OBJ_FILES = module_mail.o module_user.o smtp.o
SRC_FILES = module_mail.cpp module_user.cpp smtp.cpp

all: $(BIN_FILE)
$(BIN_FILE): $(OBJ_FILES) smtp.cpp
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(BIN_FILE) $(CFLAGS_EXTRA)
$(OBJ_FILES):
	$(CC) $(CFLAGS) -c $(SRC_FILES)

clean_object: *.o
clean:
	rm *.o $(BIN_FILE)
