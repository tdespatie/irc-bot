
CC = g++ 

TARGET = ircbot

OBJECTS = main.o irc.o win.o

all: $(TARGET)

%.o: %.c
	$(CC) -c -o $@ $<

clean: clean-obj clean-bin

clean-obj:
	rm -rf *.o
	
clean-bin:
	rm -rf $(TARGET)
	
$(TARGET): $(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS)
