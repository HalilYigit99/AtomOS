# C++ dosyaları için derleme kuralları

# C++ kaynak dosyalarını bul
CPP_SOURCES = $(shell find $(SRC_DIR) -name "*.cpp")
CPP_OBJECTS = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/%.cpp.o, $(CPP_SOURCES))

# C++ dosyaları için derleme kuralı
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@
	@echo "C++: $< -> $@"

cpp-files: $(CPP_OBJECTS)