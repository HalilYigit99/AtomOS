# C dosyaları için derleme kuralları

# C kaynak dosyalarını bul
C_SOURCES = $(shell find $(SRC_DIR) -name "*.c")
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(C_SOURCES))

# C++ özel flag'ları filtrele (C için uygun olmayan flag'lar)
CPP_ONLY_FLAGS = -fno-rtti -fno-exceptions -std=c++ -Wno-write-strings
FILTERED_CFLAGS = $(filter-out $(CPP_ONLY_FLAGS), $(CFLAGS))

# C dosyaları için derleme kuralı
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(FILTERED_CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@
	@echo "C: $< -> $@"

c-files: $(C_OBJECTS)