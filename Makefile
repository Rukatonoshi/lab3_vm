# Makefile
# Простой, но мощный Makefile для анализатора идиом

# Компилятор
CC = gcc

# Флаги
#CFLAGS = -m32 -Wall -Wextra -Werror -std=c99 -O2 -D_FILE_OFFSET_BITS=64 -Iinclude
#CFLAGS = -g2 -D_FILE_OFFSET_BITS=64 -Iinclude -fstack-protector-all
CFLAGS = -m32 -O2 -Iinclude -fstack-protector-all

# Библиотеки
LIBS = -lm

# Цель
TARGET = lama-analyzer

# Источники (все .c файлы)
SRCS = src/frequency_analyzer.c src/instructions.c

# Объектные файлы
OBJS = $(SRCS:.c=.o)

# Зависимости (включая заголовки)
DEPS = \
    include/instructions.h \
    src/byte_file.h \
    src/bytecode_decoder.h \
    src/uthash.h

# Сборка
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Компиляция .c → .o
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Очистка
clean:
	rm -f $(OBJS) $(TARGET)

# Проверка (если нужно)
.PHONY: clean

# Зависимости (автоматические)
-include $(OBJS:.o=.d)

# Генерация .d файлов (для зависимостей)
%.d: %.c $(DEPS)
	@set -e; rm -f $@; \
	$(CC) $(CFLAGS) -MM $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Включение зависимостей
-include $(OBJS:.o=.d)
