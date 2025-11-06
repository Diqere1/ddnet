# 🚀 Документация: Интеграция 4 Dummy в DDNet

## Быстрый старт

Этот проект содержит полную документацию для интеграции поддержки 4 управляемых dummy (0, 1, 2, 3) в клиент DDNet.

### 📋 Что изменилось?

- ✅ NUM_DUMMIES: 2 → 4
- ✅ Добавлены dummy 2 и 3
- ✅ Пары dummy: (0↔1) и (2↔3)
- ✅ Исправлены фризы при переключении
- ✅ Исправлены ненужные удары при свапе

---

## 📚 Документы

### Начните здесь: [INDEX_DUMMY_INTEGRATION.md](./INDEX_DUMMY_INTEGRATION.md)
Главный индекс со ссылками на все документы и картой навигации.

---

### Быстрый доступ:

| Документ | Размер | Описание | Когда использовать |
|----------|--------|----------|-------------------|
| **[INDEX_DUMMY_INTEGRATION.md](./INDEX_DUMMY_INTEGRATION.md)** | 16KB | 🗺️ Главный индекс и карта навигации | Точка входа в документацию |
| **[SUMMARY_DUMMY_2_3_CHANGES.md](./SUMMARY_DUMMY_2_3_CHANGES.md)** | 15KB | 📝 Краткое резюме всех изменений | Быстрый обзор и чек-лист |
| **[DUMMY_2_3_INTEGRATION_GUIDE.md](./DUMMY_2_3_INTEGRATION_GUIDE.md)** | 32KB | 📖 Полное руководство по интеграции | Детальная инструкция |
| **[DUMMY_2_3_CHANGES_EXAMPLES.md](./DUMMY_2_3_CHANGES_EXAMPLES.md)** | 19KB | 💻 Примеры "было → стало" | Справочник при написании кода |
| **[OnSnapInput_Integration.md](./OnSnapInput_Integration.md)** | 17KB | 🎯 Интеграция OnSnapInput | При работе с input |

---

## ⚡ Быстрая навигация

### По типу задачи:

```
"Хочу понять что делать"    → SUMMARY_DUMMY_2_3_CHANGES.md
"Готов начать интеграцию"   → DUMMY_2_3_INTEGRATION_GUIDE.md
"Нужны примеры кода"         → DUMMY_2_3_CHANGES_EXAMPLES.md
"Интегрирую OnSnapInput"     → OnSnapInput_Integration.md
"Есть проблема/ошибка"       → SUMMARY (раздел "Частые ошибки")
```

### По файлам:

```
enums.h              → GUIDE §1 + EXAMPLES §1
client.h             → GUIDE §1 + EXAMPLES §1
client/client.h      → GUIDE §1 + EXAMPLES §1-2
client/client.cpp    → GUIDE §2 + EXAMPLES §2-5, 8-9
gameclient.cpp       → GUIDE §3 + EXAMPLES §6-7 + OnSnapInput
config_variables.h   → GUIDE §4 + EXAMPLES §10
```

---

## 🎯 Основные концепции

### Пары dummy
```
Пара 1: dummy 0 ↔ dummy 1
Пара 2: dummy 2 ↔ dummy 3
```

### Формулы
```cpp
// Расчет парного dummy
if(ControlledConnection <= 1)
    PairedConnection = 1 - ControlledConnection;  // 0→1, 1→0
else
    PairedConnection = 5 - ControlledConnection;  // 2→3, 3→2

// Расчет InputConnection
InputConnection = g_Config.m_ClDummy ^ Dummy;
```

### Новые команды
```
dummy_connect          # Подключить dummy 1
dummy_connect2         # Подключить dummy 2
dummy_connect3         # Подключить dummy 3
dummy_disconnect       # Отключить dummy 1
dummy_disconnect_all   # Отключить все dummy
cl_dummy 0/1/2/3       # Переключение управления
```

---

## ✅ Чек-лист интеграции

```
□ NUM_DUMMIES = 4
□ CONN_DUMMY2/3 добавлены
□ Все массивы расширены до [4]
□ DummyConnect2/3() работают
□ DummyDisconnect() только для dummy 1
□ DummyDisconnectAll() добавлена
□ OnDummySwap() сбрасывает другую пару
□ OnSnapInput() использует InputConnection
□ Снапшоты очищаются при смене
□ MAX_SNAPSHOTS_PER_UPDATE добавлен
□ Config переменные обновлены
□ Консольные команды работают
□ Протестировано без фризов
□ Протестировано без ненужных ударов
```

---

## 📊 Статистика изменений

- **Файлов затронуто**: 6 основных
- **Функций добавлено**: 5+
- **Команд добавлено**: 3
- **Config переменных**: 3
- **Массивов расширено**: 20+
- **Строк кода**: ~2000+

---

## ⏱️ Время интеграции

| Этап | Время | Сложность |
|------|-------|-----------|
| Константы и массивы | 1-2 часа | 🟢 Легко |
| Функции подключения | 2-3 часа | 🟡 Средне |
| OnDummySwap | 2 часа | 🟡 Средне |
| OnSnapInput | 2-4 часа | 🔴 Сложно |
| Снапшоты | 1-2 часа | 🟡 Средне |
| **Итого** | **8-13 часов** | |

---

## ⚠️ Критические изменения

### 1. OnDummySwap - Сброс другой пары
```cpp
if(g_Config.m_ClDummy <= 1)
{
    m_Controls.ResetInput(2);
    m_Controls.ResetInput(3);
}
else
{
    m_Controls.ResetInput(0);
    m_Controls.ResetInput(1);
}
```

### 2. OnSnapInput - InputConnection
```cpp
int InputConnection = g_Config.m_ClDummy ^ Dummy;
int TargetConnection = /* расчет парного */;

if(InputConnection != TargetConnection)
    return 0;  // Блокировка другой пары
```

### 3. Очистка снапшотов
```cpp
m_aSnapshotStorage[g_Config.m_ClDummy].PurgeUntil(PurgeUntilTick);
```

---

## 🆘 Частые проблемы

| Проблема | Решение | Документ |
|----------|---------|----------|
| Фриз при переключении | Добавить очистку снапшотов | GUIDE §2.6 |
| Ненужные удары | Сбросить другую пару в OnDummySwap | GUIDE §3.2 |
| Input в неправильный dummy | Использовать InputConnection | OnSnapInput |
| Компиляция падает | Проверить размеры всех массивов | EXAMPLES §1 |

---

## 📞 Поддержка

### Документация создана на основе:
- ✅ Коммитов из этого репозитория
- ✅ Анализа изменений для dummy 2 и 3
- ✅ Тестирования и исправления багов

### Проверено на:
- DDNet 18.0+
- Работает с dummy 0, 1, 2, 3
- Нет фризов при переключении
- Нет ненужных ударов

---

## 🎓 Рекомендуемый порядок чтения

1. **Этот файл** (README_DUMMY_INTEGRATION.md) - вы здесь ✅
2. **INDEX_DUMMY_INTEGRATION.md** - полная карта документации
3. **SUMMARY_DUMMY_2_3_CHANGES.md** - краткий обзор
4. **DUMMY_2_3_INTEGRATION_GUIDE.md** - детальное руководство
5. **DUMMY_2_3_CHANGES_EXAMPLES.md** - примеры при написании кода
6. **OnSnapInput_Integration.md** - при интеграции input

---

## 🎉 Результат после интеграции

После выполнения всех изменений вы получите:

✅ 4 управляемых dummy (main + 3 dummy)  
✅ Правильная маршрутизация input между парами  
✅ Отсутствие фризов при переключении  
✅ Отсутствие ненужных ударов  
✅ Индивидуальное управление подключением/отключением  
✅ Полная совместимость с существующим кодом  

---

**Начните с [INDEX_DUMMY_INTEGRATION.md](./INDEX_DUMMY_INTEGRATION.md) и следуйте документации!**

*Удачи в разработке! 🚀*
