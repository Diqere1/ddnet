# Краткое резюме изменений для поддержки Dummy 2 и 3

## 📋 Обзор

Этот документ содержит краткое резюме всех изменений, необходимых для добавления поддержки 4 dummy (0, 1, 2, 3) в DDNet клиент.

---

## 📚 Доступные документы

1. **DUMMY_2_3_INTEGRATION_GUIDE.md** - Полное руководство по интеграции с детальными объяснениями
2. **DUMMY_2_3_CHANGES_EXAMPLES.md** - Конкретные примеры изменений кода в формате "было → стало"
3. **OnSnapInput_Integration.md** - Полный код функции OnSnapInput для 4 dummy

---

## 🔑 Ключевые изменения

### 1. Константы и типы

```cpp
// src/engine/client/enums.h
NUM_DUMMIES = 4  // было 2

// src/engine/client.h
enum {
    CONN_MAIN = 0,
    CONN_DUMMY = 1,
    CONN_DUMMY2 = 2,    // НОВОЕ
    CONN_DUMMY3 = 3,    // НОВОЕ
    CONN_CONTACT,
    NUM_CONNS,
};
```

### 2. Расширенные массивы

Все массивы расширены с [2] до [4] или {0, 0} до {0, 0, 0, 0}:

- `m_aPrevGameTick[NUM_DUMMIES]`
- `m_aCurGameTick[NUM_DUMMIES]`
- `m_aGameIntraTick[NUM_DUMMIES]`
- `m_aSnapshotParts[NUM_DUMMIES]`
- `m_aReceivedSnapshots[NUM_DUMMIES]`
- `m_aDummyConnected[NUM_DUMMIES]`
- `m_aDummyConnecting[NUM_DUMMIES]`
- И все остальные массивы с NUM_DUMMIES

### 3. Новые функции

```cpp
// Подключение
void DummyConnect2();
void DummyConnect3();

// Отключение
void DummyDisconnect();        // Только dummy 1
void DummyDisconnectAll();     // Все dummy

// Имя
const char *DummyName(int DummyIndex);  // Поддержка индекса 1, 2, 3
```

### 4. Новые консольные команды

```
dummy_connect          # Подключить dummy 1
dummy_connect2         # Подключить dummy 2
dummy_connect3         # Подключить dummy 3
dummy_disconnect       # Отключить dummy 1
dummy_disconnect_all   # Отключить все dummy
cl_dummy 0/1/2/3       # Переключение управления
```

### 5. Новые config переменные

```cpp
MACRO_CONFIG_INT(ClDummy, cl_dummy, 0, 0, 3, ...)  // было 0, 0, 1
MACRO_CONFIG_STR(PlayerDummy2Name, player_dummy2_name, 16, ...)
MACRO_CONFIG_STR(PlayerDummy3Name, player_dummy3_name, 16, ...)
```

---

## 🎯 Критические изменения для корректной работы

### 1. OnDummySwap() - Сброс input для другой пары

```cpp
void CGameClient::OnDummySwap()
{
    // Определение парного dummy
    int PairedConnection;
    if(g_Config.m_ClDummy <= 1)
        PairedConnection = 1 - g_Config.m_ClDummy; // 0->1, 1->0
    else
        PairedConnection = 5 - g_Config.m_ClDummy; // 2->3, 3->2

    // КРИТИЧНО: Сброс input для другой пары
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
    
    // Остальной код...
}
```

**Зачем**: Предотвращает ненужные удары/действия при переключении между парами dummy.

### 2. OnSnapInput() - Правильная маршрутизация input

```cpp
int CGameClient::OnSnapInput(int *pData, bool Dummy, bool Force)
{
    if(!Dummy)
        return m_Controls.SnapInput(pData);

    // КРИТИЧНО: Расчет InputConnection и TargetConnection
    int InputConnection = g_Config.m_ClDummy ^ Dummy;
    int ControlledConnection = g_Config.m_ClDummy;
    
    int TargetConnection;
    if(ControlledConnection <= 1)
        TargetConnection = 1 - ControlledConnection;  // 0->1, 1->0
    else
        TargetConnection = 5 - ControlledConnection;  // 2->3, 3->2

    // КРИТИЧНО: Не отправлять input для dummy из другой пары
    bool ShouldApplySpecialLogic = (InputConnection == TargetConnection);
    
    if(!g_Config.m_ClDummyHammer || !ShouldApplySpecialLogic)
    {
        if(InputConnection != TargetConnection)
            return 0;  // Блокировка input для другой пары
        
        // ... обычный input
    }
    
    // ... hammer логика
}
```

**Зачем**: 
- Правильная маршрутизация input между парами (0↔1 и 2↔3)
- Предотвращение отправки input в неправильные dummy

### 3. CClient::Update() - Очистка снапшотов

```cpp
if(m_LastDummy != (bool)g_Config.m_ClDummy)
{
    GameClient()->InvalidateSnapshot();
    GameClient()->OnDummySwap();

    // КРИТИЧНО: Очистка старых накопленных снапшотов
    if(m_aapSnapshots[g_Config.m_ClDummy][SNAP_CURRENT])
    {
        CSnapshotStorage::CHolder *pLastHolder = m_aSnapshotStorage[g_Config.m_ClDummy].m_pLast;
        if(pLastHolder)
        {
            int PurgeUntilTick = pLastHolder->m_Tick - 5;
            if(PurgeUntilTick > 0)
            {
                m_aSnapshotStorage[g_Config.m_ClDummy].PurgeUntil(PurgeUntilTick);
                m_aapSnapshots[g_Config.m_ClDummy][SNAP_CURRENT] = m_aSnapshotStorage[g_Config.m_ClDummy].m_pLast;
                m_aapSnapshots[g_Config.m_ClDummy][SNAP_PREV] = /* ... */;
                // Обновление тиков...
            }
        }
    }
}
```

**Зачем**: Предотвращает фриз игры при переключении на dummy 2/3 из-за накопленных снапшотов.

### 4. Ограничение обработки снапшотов

```cpp
// В циклах обработки снапшотов
int ProcessedSnapshots = 0;
const int MAX_SNAPSHOTS_PER_UPDATE = 10;

while(true)
{
    // ... обработка снапшота
    
    ProcessedSnapshots++;
    if(ProcessedSnapshots >= MAX_SNAPSHOTS_PER_UPDATE)
        break;
}
```

**Зачем**: Дополнительная защита от фриза при большом количестве накопленных снапшотов.

---

## 📊 Статистика изменений

### Затронутые файлы

| Файл | Тип изменений | Важность |
|------|---------------|----------|
| `src/engine/client/enums.h` | NUM_DUMMIES: 2→4 | 🔴 Критично |
| `src/engine/client.h` | Новые enum, методы, массивы | 🔴 Критично |
| `src/engine/client/client.h` | Массивы, методы, переменные | 🔴 Критично |
| `src/engine/client/client.cpp` | Логика подключения, снапшоты | 🔴 Критично |
| `src/game/client/gameclient.cpp` | OnDummySwap, OnSnapInput | 🔴 Критично |
| `src/engine/shared/config_variables.h` | cl_dummy, имена dummy | 🟡 Важно |

### Количественные показатели

- **Функций добавлено**: 5+ (DummyConnect2/3, DummyDisconnectAll, и т.д.)
- **Консольных команд**: 3 новых
- **Config переменных**: 3 новых
- **Массивов расширено**: 20+
- **Строк кода изменено**: ~2000+

---

## 🚀 Порядок интеграции

### Этап 1: Базовые изменения (обязательно)
1. ✅ Изменить NUM_DUMMIES на 4
2. ✅ Добавить CONN_DUMMY2 и CONN_DUMMY3
3. ✅ Расширить все массивы до [4]

### Этап 2: Функции подключения (обязательно)
4. ✅ Добавить DummyConnect2() и DummyConnect3()
5. ✅ Изменить DummyDisconnect() на отключение только dummy 1
6. ✅ Добавить DummyDisconnectAll()
7. ✅ Обновить DummyConnected() и DummyConnecting()

### Этап 3: Input и снапшоты (критично!)
8. ✅ Обновить OnDummySwap() со сбросом другой пары
9. ✅ Полностью переписать OnSnapInput() с InputConnection
10. ✅ Добавить логику очистки снапшотов в Update()
11. ✅ Добавить ограничение обработки снапшотов

### Этап 4: Config и команды (важно)
12. ✅ Обновить cl_dummy до 0-3
13. ✅ Добавить player_dummy2_name и player_dummy3_name
14. ✅ Зарегистрировать новые консольные команды

### Этап 5: Тестирование
15. ✅ Проверить подключение всех dummy
16. ✅ Проверить переключение без фризов
17. ✅ Проверить отсутствие ненужных ударов
18. ✅ Проверить работу hammer для всех пар

---

## ⚠️ Частые ошибки

### 1. Использование !g_Config.m_ClDummy
❌ **Неправильно**: `m_aLocalIds[!g_Config.m_ClDummy]`  
✅ **Правильно**: `m_aLocalIds[InputConnection]`

### 2. Хардкод размера массива
❌ **Неправильно**: `int m_aData[2];`  
✅ **Правильно**: `int m_aData[NUM_DUMMIES];`

### 3. Отключение всех dummy вместо одного
❌ **Неправильно**: DummyDisconnect() отключает всех  
✅ **Правильно**: DummyDisconnect() → только dummy 1, DummyDisconnectAll() → все

### 4. Неправильный расчет парного dummy
❌ **Неправильно**: `1 - g_Config.m_ClDummy` для всех  
✅ **Правильно**: 
```cpp
if(g_Config.m_ClDummy <= 1)
    PairedConnection = 1 - g_Config.m_ClDummy;
else
    PairedConnection = 5 - g_Config.m_ClDummy;
```

### 5. Забыть очистить снапшоты при смене
❌ **Результат**: Фриз игры при переключении  
✅ **Решение**: Добавить PurgeUntil в OnDummySwap

---

## 🔍 Команды для проверки

```bash
# Найти все места с NUM_DUMMIES
grep -rn "NUM_DUMMIES" src/

# Найти использование !g_Config.m_ClDummy
grep -rn "!g_Config.m_ClDummy" src/game/client/

# Найти массивы размером [2]
grep -rn "\[2\]" src/engine/client/client.h

# Проверить инициализацию массивов
grep -rn "{0, 0}" src/engine/client/
grep -rn "{false, false}" src/engine/client/
```

---

## 📖 Формулы и логика

### Расчет парного dummy
```
Пара 1: dummy 0 ↔ dummy 1
    PairedConnection = 1 - ControlledConnection
    0 → 1, 1 → 0

Пара 2: dummy 2 ↔ dummy 3
    PairedConnection = 5 - ControlledConnection
    2 → 3, 3 → 2
```

### Расчет InputConnection
```
InputConnection = g_Config.m_ClDummy ^ Dummy
где Dummy = true (1) для dummy input
```

Примеры:
- Управляем dummy 0, input для dummy: 0 ^ 1 = 1 (dummy 1)
- Управляем dummy 1, input для dummy: 1 ^ 1 = 0 (dummy 0)
- Управляем dummy 2, input для dummy: 2 ^ 1 = 3 (dummy 3)
- Управляем dummy 3, input для dummy: 3 ^ 1 = 2 (dummy 2)

---

## ✅ Финальный чек-лист

Перед завершением убедитесь:

- [ ] NUM_DUMMIES = 4
- [ ] Все массивы расширены до [4] или {0,0,0,0}
- [ ] CONN_DUMMY2 и CONN_DUMMY3 добавлены
- [ ] DummyConnect2() и DummyConnect3() работают
- [ ] DummyDisconnect() отключает только dummy 1
- [ ] DummyDisconnectAll() добавлена и работает
- [ ] OnDummySwap() сбрасывает input другой пары
- [ ] OnSnapInput() использует InputConnection
- [ ] OnSnapInput() блокирует input для другой пары
- [ ] Снапшоты очищаются при смене dummy
- [ ] Добавлено ограничение MAX_SNAPSHOTS_PER_UPDATE
- [ ] cl_dummy поддерживает 0-3
- [ ] player_dummy2_name и player_dummy3_name добавлены
- [ ] Консольные команды зарегистрированы
- [ ] Нет использования !g_Config.m_ClDummy в OnSnapInput
- [ ] Все циклы используют NUM_DUMMIES
- [ ] Протестировано подключение всех dummy
- [ ] Протестировано переключение без фризов
- [ ] Протестировано отсутствие ненужных ударов

---

## 📞 Помощь и дополнительная информация

### Полная документация
- **DUMMY_2_3_INTEGRATION_GUIDE.md** - детальное руководство по каждому файлу
- **DUMMY_2_3_CHANGES_EXAMPLES.md** - примеры изменений "было → стало"
- **OnSnapInput_Integration.md** - полная реализация OnSnapInput

### Ключевые концепции
1. **Пары dummy**: (0↔1) и (2↔3)
2. **InputConnection**: рассчитывается через XOR
3. **TargetConnection**: парный dummy для текущего
4. **ShouldApplySpecialLogic**: применять ли hammer только для парного
5. **Очистка снапшотов**: предотвращает фризы

---

## 🎉 Итоги

После выполнения всех изменений вы получите:

✅ **4 управляемых dummy** (main + 3 dummy)  
✅ **Правильная маршрутизация input** между парами  
✅ **Отсутствие фризов** при переключении  
✅ **Отсутствие ненужных ударов** при переключении между парами  
✅ **Индивидуальное управление** подключением/отключением  
✅ **Совместимость** с существующим кодом

Следуйте руководству последовательно, тестируйте каждый этап, и интеграция пройдет гладко!
