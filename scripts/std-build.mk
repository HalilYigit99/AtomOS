# Standard kütüphane derleme kuralları

STD_C_SOURCES = $(shell find $(STD_DIR)/src -name "*.c" 2>/dev/null || true)
STD_CPP_SOURCES = $(shell find $(STD_DIR)/src -name "*.cpp" 2>/dev/null || true)
STD_C_OBJECTS = $(patsubst $(STD_DIR)/src/%.c, $(STD_DIR)/build/%.c.o, $(STD_C_SOURCES))
STD_CPP_OBJECTS = $(patsubst $(STD_DIR)/src/%.cpp, $(STD_DIR)/build/%.cpp.o, $(STD_CPP_SOURCES))

# STD kütüphanesi için özel derleyici bayrakları (userspace için)
STD_CFLAGS = -ffreestanding -fPIC -O2 -Wall -Wextra

# STD C dosyaları
$(STD_DIR)/build/%.c.o: $(STD_DIR)/src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(STD_CFLAGS) -I$(STD_DIR)/include -c $< -o $@
	@echo "STD-C: $< -> $@"

# STD C++ dosyaları
$(STD_DIR)/build/%.cpp.o: $(STD_DIR)/src/%.cpp
	@mkdir -p $(dir $@)
	$(CC) $(STD_CFLAGS) -I$(STD_DIR)/include -c $< -o $@
	@echo "STD-C++: $< -> $@"

# STD kütüphanesi oluştur
$(STD_DIR)/stdlib.so: $(STD_C_OBJECTS) $(STD_CPP_OBJECTS)
	@if [ -n "$(STD_C_OBJECTS)$(STD_CPP_OBJECTS)" ]; then \
		$(CC) -shared -o $@ $(STD_C_OBJECTS) $(STD_CPP_OBJECTS); \
		echo "STD kütüphanesi oluşturuldu: $@"; \
	else \
		echo "STD kaynak dosyası bulunamadı, boş kütüphane oluşturuluyor."; \
		touch $@; \
	fi

std-build: $(STD_DIR)/stdlib.so