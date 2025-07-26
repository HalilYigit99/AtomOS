# C dosyaları için derleme kuralları

# C kaynak dosyalarını bul
C_SOURCES = $(shell find $(SRC_DIR) -name "*.c")
C_OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(C_SOURCES))

# C dosyaları için derleme kuralı
$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@
	@echo "C: $< -> $@"

c-files: $(C_OBJECTS)