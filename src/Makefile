CC          =
CFLAGS      = -Wall -Wextra -O2
DEBUG_CFLAGS = -Wall -Wextra -Werror -g -O0 -DDEBUG=1
LDFLAGS     =

SRCDIR      = .
BUILDDIR    = build
TARGET      = maze

LIB_SOURCES = cave.c \
              cli.c \
              maze.c \
              solver.c \
              utils.c \
			  generate_maze.c \
			  position.c \
			  path.c \
			  queue.c \
			  rl_solver.c \
			  generate_cave.c

MAIN_SOURCE = main.c

LIB_TARGET  = libmaze.a
LIB_OBJECTS  = $(LIB_SOURCES:%.c=$(BUILDDIR)/%.o)
MAIN_OBJECT  = $(MAIN_SOURCE:%.c=$(BUILDDIR)/%.o)

TEST_DIR     = tests
TEST_SOURCES = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJECTS = $(TEST_SOURCES:%.c=$(BUILDDIR)/%.o)
TEST_TARGET  = $(BUILDDIR)/test_runner
CHECK_LIBS = $(shell pkg-config --libs check 2>/dev/null || echo "-lcheck -lm -lpthread -lsubunit")

WS_DIR = webserver
WS_SOURCES = $(wildcard $(WS_DIR)/*.c)
WS_OBJECTS = $(WS_SOURCES:%.c=$(BUILDDIR)/%.o)
WS_TARGET = $(WS_DIR)/backend.fcgi
WS_LIBS = $(shell pkg-config --libs fcgi xxhash 2>/dev/null || echo "-lfcgi -lxxhash")

GCOV_INFO_FILE=gcovreport.info
REPORT_FOLDER=report

GCOV_DIR      = $(BUILDDIR)/gcov
GCOVFLAGS     = --coverage -O0

.PHONY: all clean install uninstall test lib web debug_web format linter valgrind release debug help re gcov_report

all: $(TARGET) lib $(WS_TARGET)
	@echo "Build completed successfully"

$(BUILDDIR):
	@mkdir -p $(BUILDDIR) $(BUILDDIR)/$(TEST_DIR)/ $(BUILDDIR)/$(WS_DIR)/

$(TARGET): $(BUILDDIR) $(LIB_OBJECTS) $(MAIN_OBJECT)
	@$(CC) $(LDFLAGS) -o $@ $(LIB_OBJECTS) $(MAIN_OBJECT)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(BUILDDIR)
	@$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

$(BUILDDIR)/$(WS_DIR)/%.o: $(WS_DIR)/%.c | $(BUILDDIR)
	@$(CC) $(CFLAGS) -I$(SRCDIR) -c $< -o $@

PREFIX ?= /usr/local

install: $(TARGET)
	@echo "Installing $(TARGET) to $(PREFIX)/bin"
	install -Dm755 $(TARGET) $(PREFIX)/bin/$(TARGET)

uninstall:
	@echo "Removing $(TARGET) from $(PREFIX)/bin"
	rm -f $(PREFIX)/bin/$(TARGET)

lib: $(BUILDDIR) $(LIB_OBJECTS)
	@ar rcs $(LIB_TARGET) $(LIB_OBJECTS)
	@echo "Built static library: $(LIB_TARGET)"

web:
	@which docker || echo "Docker not found. Docker-CLI is required for webserver"
	@docker rm -f A1_Maze_C 
	@docker build --target release -t maze-server-ht ./$(WS_DIR)
	@docker run --name A1_Maze_C --restart unless-stopped -d -p 8081:81 maze-server-ht

backend: 
	@which docker || echo "Docker not found. Docker-CLI is required for webserver"
	@docker build -t maze-backend-build . && docker run --rm -v `pwd`:/wrksrc maze-backend-build
	@docker rm -f maze
	@docker build --target minimal -t maze-backend-slim ./$(WS_DIR)
	@docker run -it --name maze --restart unless-stopped -d -p 127.0.0.1:8080:8080 maze-backend-slim
	@echo backend available on localhost:8080

debug_web: debug
	@which docker || echo "Docker not found. Docker-CLI is required for webserver"
	@docker rm -f A1_Maze_C
	@docker build --target debug -t maze-server-ht-debug ./$(WS_DIR)
	@docker run --name A1_Maze_C -d -p 8081:81 maze-server-ht-debug
	@docker logs A1_Maze_C -f

clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILDDIR) $(LIB_TARGET) $(TARGET) $(GCOV_INFO_FILE) $(REPORT_FOLDER) $(WS_DIR)/backend.fcgi

$(TEST_TARGET): $(BUILDDIR) $(TEST_OBJECTS) $(LIB_OBJECTS)
	@$(CC) -o $@ $(TEST_OBJECTS) $(LIB_OBJECTS) $(CHECK_LIBS)

$(WS_TARGET): $(BUILDDIR) $(WS_OBJECTS) $(LIB_OBJECTS)
	@$(CC) -o $@ $(WS_OBJECTS) $(LIB_OBJECTS) $(WS_LIBS)

test: $(TEST_TARGET)
	@echo "Running tests..."
	@$(TEST_TARGET)

$(GCOV_DIR):
	@mkdir -p $(GCOV_DIR) $(GCOV_DIR)/$(TEST_DIR)

$(GCOV_DIR)/%.o: $(SRCDIR)/%.c | $(GCOV_DIR)
	@$(CC) $(CFLAGS) $(GCOVFLAGS) -c $< -o $@

$(GCOV_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.c | $(GCOV_DIR)
	@$(CC) $(CFLAGS) $(GCOVFLAGS) -I$(SRCDIR) -c $< -o $@

GCOV_LIB_OBJECTS  = $(LIB_SOURCES:%.c=$(GCOV_DIR)/%.o)
GCOV_TEST_OBJECTS = $(TEST_SOURCES:%.c=$(GCOV_DIR)/%.o)
GCOV_TEST_TARGET  = $(GCOV_DIR)/test_runner

$(GCOV_TEST_TARGET): $(GCOV_DIR) $(GCOV_TEST_OBJECTS) $(GCOV_LIB_OBJECTS)
	@$(CC) $(GCOVFLAGS) -o $@ $(GCOV_TEST_OBJECTS) $(GCOV_LIB_OBJECTS) $(CHECK_LIBS)

gcov_report: $(GCOV_TEST_TARGET)
	@echo "Running tests with coverage instrumentation..."
	@$(GCOV_TEST_TARGET)
	@echo "Collecting coverage data..."
	@lcov --capture --directory $(GCOV_DIR) \
	      --output-file $(GCOV_INFO_FILE) \
	      --exclude '*/tests/*' \
	      --ignore-errors unused
	@echo "Generating HTML report..."
	@genhtml $(GCOV_INFO_FILE) --output-directory $(REPORT_FOLDER)
	@echo "Coverage report ready: $(REPORT_FOLDER)/index.html"

re: clean all

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

release: clean
	$(MAKE) CFLAGS="$(CFLAGS)" LDFLAGS="-s" all

debug: clean
	$(MAKE) CFLAGS="$(DEBUG_CFLAGS)" LDFLAGS="$(LDFLAGS)" all

