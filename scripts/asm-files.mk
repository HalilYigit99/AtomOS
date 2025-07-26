# Assembly dosyaları için derleme kuralları

# Assembly kaynak dosyalarını bul (.asm, .s, .S)
ASM_SOURCES = $(shell find $(SRC_DIR) -name "*.asm" -o -name "*.s" -o -name "*.S")
ASM_OBJECTS = $(patsubst $(SRC_DIR)/%.asm, $(BUILD_DIR)/%.asm.o, \
              $(patsubst $(SRC_DIR)/%.s, $(BUILD_DIR)/%.s.o, \
              $(patsubst $(SRC_DIR)/%.S, $(BUILD_DIR)/%.S.o, $(ASM_SOURCES))))

# Assembly dosyaları için derleme kuralları
$(BUILD_DIR)/%.asm.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@
	@echo "ASM: $< -> $@"

$(BUILD_DIR)/%.s.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@
	@echo "ASM: $< -> $@"

$(BUILD_DIR)/%.S.o: $(SRC_DIR)/%.S
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@
	@echo "ASM: $< -> $@"

asm-files: $(ASM_OBJECTS)