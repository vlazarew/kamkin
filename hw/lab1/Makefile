SOURCE=test.cpp
TARGET=test

CFLAGS = -I. -Wall

all: $(TARGET)

$(TARGET): $(SOURCE)
	g++ $(CFLAGS) -o $@ -I. $^

run:
	./$(TARGET)

clean:
	rm -rf $(TARGET)

