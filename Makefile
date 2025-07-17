PROJECT_NAME := InfotexContest
BUILD_DIR := build
LOG_APP := $(BUILD_DIR)/log_app/log_app
STATS_APP := $(BUILD_DIR)/stat_app/stats_collector

LOG_FILE := logs.txt
LOG_SOCKET := socket:127.0.0.1:8000
DEFAULT_LOG_LEVEL := Mid

STATS_PORT := 8000
MSG_THRESHOLD := 3
TIMEOUT_SEC := 10

USE_SHARED ?= ON

.PHONY: all build run stats socket-run socket-listen clean rebuild test

all: rebuild run

build:
	@echo "Building project..."
	@mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake .. -DUSE_SHARED=$(USE_SHARED)
	$(MAKE) -C $(BUILD_DIR)

run:
	@echo "Running Log Application in file mode..."
	@$(LOG_APP) $(LOG_FILE) $(DEFAULT_LOG_LEVEL)

stats:
	@echo "Starting Stats Collector on port $(STATS_PORT)..."
	@$(STATS_APP) $(STATS_PORT) $(MSG_THRESHOLD) $(TIMEOUT_SEC)

socket-run:
	@echo "Running Log Application in socket mode..."
	@$(LOG_APP) $(LOG_SOCKET) $(DEFAULT_LOG_LEVEL)

socket-listen:
	@echo "Listening on port 8000..."
	@nc -l 8000

clean:
	@echo "Cleaning build files and logs..."
	rm -rf $(BUILD_DIR)
	rm -f $(LOG_FILE)

rebuild: clean build

test: 
	cd build && ctest --output-on-failure


