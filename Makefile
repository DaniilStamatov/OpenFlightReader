CXX = g++
CXXFLAGS = -std=c++17 -Wall -Werror -Wextra
TARGET = openflight_reader
SOURCES = main.cc OpenFlightReader.cc

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: clean run