# Примеры сгенерированного кода из X-макросов

## Как посмотреть сгенерированный код

Команда:
```bash
gcc -E -Iinclude src/instructions.c > generated_code.i
```

Эта команда запускает препроцессор C и сохраняет результат в файл. Препроцессор расширяет все макросы и включает все файлы.

Файл содержит:
- Раскрытые макросы
- Включённые заголовочные файлы
- Сгенерированные структуры
- Маркеры строк препроцессора (начинаются с `#`)

## Сгенерированные структуры

### 1. Массивы полей формата (из BEGIN_FORMAT блоков)

**Вход в opcodes.def:**
```c
BEGIN_FORMAT(closure_format)
FORMAT_FIELD(FIELD_OP, 4, -1)   // ip: 4-byte fixed field
FORMAT_FIELD(FIELD_COUNT, 4, 0) // n: count field, store at index 0
FORMAT_FIELD(FIELD_OP, 5, 0)    // varspec: 5-byte field, repeat count[0] times
FORMAT_END
```

**Сгенерированный код:**
```c
static const FieldDesc closure_format_fields[] = {
{ 0, 4, -1 },      // FIELD_OP, size=4, repeat_from=-1
{ 1, 4, 0 },       // FIELD_COUNT, size=4, repeat_from=0
{ 0, 5, 0 },       // FIELD_OP, size=5, repeat_from=0
};
```

### 2. Дескрипторы формата (из VARLEN_DESCRIPTOR)

**Вход в opcodes.def:**
```c
VARLEN_DESCRIPTOR(54, closure, 0, closure_format, 3)
```

**Сгенерированный код:**
```c
static const FormatDesc closure_format_desc = {
    .fields = closure_format_fields,
    .field_count = 3
};
```

### 3. Таблица инструкций (из INSTR и VARLEN_DESCRIPTOR)

**Вход в opcodes.def:**
```c
INSTR(10, const, 4, 0)           // CONST
INSTR(15, jmp, 4, 0x07)          // JMP (JUMP|HALT|BREAK)
VARLEN_DESCRIPTOR(54, closure, 0, closure_format, 3)  // CLOSURE
INSTR(55, callc, 4, 0x04)        // CALLC
```

**Сгенерированный код:**
```c
{ 0x10, "const", 4, 0, .length_fn = default_instruction_length, .is_varlen = false },
{ 0x15, "jmp", 4, 0x07, .length_fn = default_instruction_length, .is_varlen = false },
{ 0x54, "closure", 0, 0, .format_desc = &closure_format_desc, .is_varlen = true },
{ 0x55, "callc", 4, 0x04, .length_fn = default_instruction_length, .is_varlen = false },
```

## Три этапа генерации

Препроцессор включает `opcodes.def` три раза с разными макросами:

1. **Первое включение**:
   - Макросы: `BEGIN_FORMAT`, `FORMAT_FIELD`, `FORMAT_END`
   - Результат: сгенерированы массивы полей (`closure_format_fields[]`)

2. **Второе включение**:
   - Макрос: `VARLEN_DESCRIPTOR` с определением format desc
   - Результат: сгенерированы дескрипторы форматов (`closure_format_desc`)

3. **Третье включение**:
   - Макросы: `INSTR`, `VARLEN_DESCRIPTOR` с определением instruction table
   - Результат: сгенерирована таблица инструкций (`instruction_table[]`)

