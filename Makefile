CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread -fPIC -O2
INCDIR = include
SRCDIR = src
LIBDIR = lib
BINDIR = bin
TESTDIR = tests

LIB_TARGET = $(LIBDIR)/liblogger.so
APP_TARGET = $(BINDIR)/app
TEST_TARGET = $(BINDIR)/test

.PHONY: all clean library app test run

all: library app test

$(LIBDIR) $(BINDIR):
	mkdir -p $@

$(LIB_TARGET): $(SRCDIR)/logger.cpp $(INCDIR)/logger.h | $(LIBDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $(SRCDIR)/logger.cpp -o $(LIBDIR)/logger.o
	$(CXX) -shared $(LIBDIR)/logger.o -o $@

library: $(LIB_TARGET)

$(APP_TARGET): $(SRCDIR)/main.cpp $(LIB_TARGET) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -L$(LIBDIR) $(SRCDIR)/main.cpp -llogger -o $@

app: $(APP_TARGET)

$(TEST_TARGET): $(TESTDIR)/test_logger.cpp $(LIB_TARGET) | $(BINDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -L$(LIBDIR) $(TESTDIR)/test_logger.cpp -llogger -o $@

test: $(TEST_TARGET)
	@echo "Running tests..."
	LD_LIBRARY_PATH=$(LIBDIR) $(TEST_TARGET)

run: app
	LD_LIBRARY_PATH=$(LIBDIR) $(APP_TARGET) app.log INFO

clean:
	rm -rf $(LIBDIR) $(BINDIR)
	rm -f *.log