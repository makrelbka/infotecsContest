BIN_PATH = build/log_app/log_app

BIN_PATH_STAT = build/stat_app/stats_collector

LOG_FILE = logs.txt
LOG_SOCKET = socket:127.0.0.1:8000
LOG_LEVEL = Mid

USE_SHARED ?= ON

PORT = 8000
N = 3
T = 10

all: rebuild run

build:
	@mkdir -p build
	cd build && cmake .. -DUSE_SHARED=$(USE_SHARED)
	$(MAKE) -C build


runStats: rebuild
	@$(BIN_PATH_STAT) $(PORT) $(N) $(T)


run: rebuild
	@$(BIN_PATH) $(LOG_FILE) $(LOG_LEVEL)

clean:
	rm -rf build
	rm -f $(LOG_FILE)

test: rebuild
	cd build && ctest --output-on-failure

rebuild: clean build

listenSocket:
	@echo "Listening on port 8000..."
	@nc -l 8000

socketRun: rebuild
	@$(BIN_PATH) $(LOG_SOCKET) $(LOG_LEVEL)


.PHONY: build run clean rebuild listenSocket
