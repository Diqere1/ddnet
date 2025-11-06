# Примеры изменений кода для поддержки Dummy 2 и 3

Этот файл содержит конкретные примеры изменений в формате "было → стало" для упрощения интеграции.

---

## 1. Расширение массивов

### Пример 1: Массивы состояния игры

**БЫЛО:**
```cpp
int m_aPrevGameTick[NUM_DUMMIES] = {0, 0};
int m_aCurGameTick[NUM_DUMMIES] = {0, 0};
float m_aGameIntraTick[NUM_DUMMIES] = {0.0f, 0.0f};
```

**СТАЛО:**
```cpp
int m_aPrevGameTick[NUM_DUMMIES] = {0, 0, 0, 0};
int m_aCurGameTick[NUM_DUMMIES] = {0, 0, 0, 0};
float m_aGameIntraTick[NUM_DUMMIES] = {0.0f, 0.0f, 0.0f, 0.0f};
```

### Пример 2: Массивы снапшотов

**БЫЛО:**
```cpp
uint64_t m_aSnapshotParts[NUM_DUMMIES] = {0, 0};
int m_aReceivedSnapshots[NUM_DUMMIES] = {0, 0};
```

**СТАЛО:**
```cpp
uint64_t m_aSnapshotParts[NUM_DUMMIES] = {0, 0, 0, 0};
int m_aReceivedSnapshots[NUM_DUMMIES] = {0, 0, 0, 0};
```

### Пример 3: Массивы состояния dummy

**БЫЛО:**
```cpp
bool m_aDummyConnected[NUM_DUMMIES] = {false, false};
bool m_aDummyConnecting[NUM_DUMMIES] = {false, false};
char m_aTimeoutCodes[NUM_DUMMIES][32] = {"", ""};
```

**СТАЛО:**
```cpp
bool m_aDummyConnected[NUM_DUMMIES] = {false, false, false, false};
bool m_aDummyConnecting[NUM_DUMMIES] = {false, false, false, false};
char m_aTimeoutCodes[NUM_DUMMIES][32] = {"", "", "", ""};
```

---

## 2. Изменения в циклах

### Пример 1: Итерация по всем dummy

**БЫЛО:**
```cpp
for(int i = 0; i < 2; i++)
{
    m_aapSnapshots[i][SNAP_CURRENT] = nullptr;
    m_aapSnapshots[i][SNAP_PREV] = nullptr;
}
```

**СТАЛО:**
```cpp
for(int i = 0; i < NUM_DUMMIES; i++)
{
    m_aapSnapshots[i][SNAP_CURRENT] = nullptr;
    m_aapSnapshots[i][SNAP_PREV] = nullptr;
}
```

### Пример 2: Отключение всех dummy

**БЫЛО:**
```cpp
void CClient::DummyDisconnect(const char *pReason)
{
    if(m_DummyConnected)
    {
        m_aNetClient[CONN_DUMMY].Disconnect(pReason);
        // ...
        m_DummyConnected = false;
    }
}
```

**СТАЛО:**
```cpp
void CClient::DummyDisconnect(const char *pReason)
{
    // Отключить только dummy 1
    if(m_aDummyConnected[1] || m_aDummyConnecting[1])
    {
        m_aNetClient[CONN_DUMMY].Disconnect(pReason);
        m_aRconAuthed[1] = 0;
        m_aapSnapshots[1][SNAP_CURRENT] = nullptr;
        m_aapSnapshots[1][SNAP_PREV] = nullptr;
        m_aReceivedSnapshots[1] = 0;
        m_aDummyConnected[1] = false;
        m_aDummyConnecting[1] = false;
        
        if(g_Config.m_ClDummy == 1)
        {
            g_Config.m_ClDummy = 0;
        }
        
        GameClient()->OnDummyDisconnect();
    }
}

void CClient::DummyDisconnectAll(const char *pReason)
{
    // Отключить все dummy
    for(int i = 1; i <= 3; i++)
    {
        if(m_aDummyConnected[i] || m_aDummyConnecting[i])
        {
            int ConnIndex = CONN_MAIN + i;
            m_aNetClient[ConnIndex].Disconnect(pReason);
            m_aRconAuthed[i] = 0;
            m_aapSnapshots[i][SNAP_CURRENT] = nullptr;
            m_aapSnapshots[i][SNAP_PREV] = nullptr;
            m_aReceivedSnapshots[i] = 0;
            m_aDummyConnected[i] = false;
            m_aDummyConnecting[i] = false;
        }
    }
    
    g_Config.m_ClDummy = 0;
    m_DummyReconnectOnReload = false;
    m_DummyDeactivateOnReconnect = false;
    GameClient()->OnDummyDisconnect();
}
```

---

## 3. Проверки состояния dummy

### Пример 1: Проверка подключения

**БЫЛО:**
```cpp
bool CClient::DummyConnected() const
{
    return m_DummyConnected;
}
```

**СТАЛО:**
```cpp
bool CClient::DummyConnected() const
{
    for(int i = 1; i <= 3; i++)
    {
        if(m_aDummyConnected[i])
            return true;
    }
    return false;
}
```

### Пример 2: Проверка процесса подключения

**БЫЛО:**
```cpp
bool CClient::DummyConnecting() const
{
    return m_DummyConnecting;
}
```

**СТАЛО:**
```cpp
bool CClient::DummyConnecting() const
{
    for(int i = 1; i <= 3; i++)
    {
        if(m_aDummyConnecting[i])
            return true;
    }
    return false;
}
```

---

## 4. Получение имени dummy

### БЫЛО:
```cpp
const char *CClient::DummyName()
{
    if(g_Config.m_PlayerDummyName[0])
        return g_Config.m_PlayerDummyName;
    str_format(m_aAutomaticDummyName, sizeof(m_aAutomaticDummyName), "[D] %s", PlayerName());
    return m_aAutomaticDummyName;
}
```

### СТАЛО:
```cpp
const char *CClient::DummyName(int DummyIndex)
{
    if(DummyIndex == 1)
    {
        if(g_Config.m_PlayerDummyName[0])
            return g_Config.m_PlayerDummyName;
        str_format(m_aAutomaticDummyName, sizeof(m_aAutomaticDummyName), "[D] %s", PlayerName());
        return m_aAutomaticDummyName;
    }
    else if(DummyIndex == 2)
    {
        if(g_Config.m_PlayerDummy2Name[0])
            return g_Config.m_PlayerDummy2Name;
        str_format(m_aAutomaticDummy2Name, sizeof(m_aAutomaticDummy2Name), "[D2] %s", PlayerName());
        return m_aAutomaticDummy2Name;
    }
    else if(DummyIndex == 3)
    {
        if(g_Config.m_PlayerDummy3Name[0])
            return g_Config.m_PlayerDummy3Name;
        str_format(m_aAutomaticDummy3Name, sizeof(m_aAutomaticDummy3Name), "[D3] %s", PlayerName());
        return m_aAutomaticDummy3Name;
    }
    return "";
}
```

---

## 5. Обработка сообщений сервера

### Пример: NETMSG_CON_READY для dummy

**БЫЛО:**
```cpp
else if(Conn == CONN_DUMMY && Msg == NETMSG_CON_READY)
{
    m_DummyConnected = true;
    m_DummyConnecting = false;
    g_Config.m_ClDummy = 1;
    // ...
}
```

**СТАЛО:**
```cpp
else if((Conn == CONN_DUMMY || Conn == CONN_DUMMY2 || Conn == CONN_DUMMY3) && Msg == NETMSG_CON_READY)
{
    int DummyIndex = Conn - CONN_MAIN;
    m_aDummyConnected[DummyIndex] = true;
    m_aDummyConnecting[DummyIndex] = false;
    g_Config.m_ClDummy = DummyIndex;
    Rcon("crashmeplx");
    if(m_aRconAuthed[0] && !m_aRconAuthed[DummyIndex])
        RconAuth(m_aRconUsername, m_aRconPassword);
}
```

---

## 6. Функция OnDummySwap

### БЫЛО:
```cpp
void CGameClient::OnDummySwap()
{
    if(g_Config.m_ClDummyResetOnSwitch)
    {
        m_Controls.ResetInput(!g_Config.m_ClDummy);
        m_Controls.m_aInputData[!g_Config.m_ClDummy].m_Hook = 0;
    }

    m_DummyFire = 0;
    m_HammerInput.m_Fire &= INPUT_STATE_MASK;
    if((m_HammerInput.m_Fire & 1) != 0)
        m_HammerInput.m_Fire++;

    int tmp = m_DummyInput.m_Fire;
    m_DummyInput = m_Controls.m_aInputData[!g_Config.m_ClDummy];
    m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire = tmp;
    
    m_DummyInput.m_Fire &= INPUT_STATE_MASK;
    if((m_DummyInput.m_Fire & 1) != 0)
        m_DummyInput.m_Fire++;
    
    m_IsDummySwapping = 1;
}
```

### СТАЛО:
```cpp
void CGameClient::OnDummySwap()
{
    // Определяем парный dummy
    int PairedConnection;
    if(g_Config.m_ClDummy <= 1)
    {
        PairedConnection = 1 - g_Config.m_ClDummy; // 0->1, 1->0
    }
    else
    {
        PairedConnection = 5 - g_Config.m_ClDummy; // 2->3, 3->2
    }

    if(g_Config.m_ClDummyResetOnSwitch)
    {
        int PlayerOrDummy = (g_Config.m_ClDummyResetOnSwitch == 2) ? g_Config.m_ClDummy : PairedConnection;
        m_Controls.ResetInput(PlayerOrDummy);
        m_Controls.m_aInputData[PlayerOrDummy].m_Hook = 0;
    }

    m_DummyFire = 0;
    m_HammerInput.m_Fire &= INPUT_STATE_MASK;
    if((m_HammerInput.m_Fire & 1) != 0)
        m_HammerInput.m_Fire++;

    // Сброс input для другой пары dummy
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

    // Для dummy 2/3 не копируем input
    if(g_Config.m_ClDummy >= 2)
    {
        m_DummyInput = {};
        m_DummyInput.m_Fire = 0;
        m_Controls.ResetInput(g_Config.m_ClDummy);
    }
    else
    {
        int tmp = m_DummyInput.m_Fire;
        m_DummyInput = m_Controls.m_aInputData[PairedConnection];
        m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire = tmp;
    }
    
    m_DummyInput.m_Fire &= INPUT_STATE_MASK;
    if((m_DummyInput.m_Fire & 1) != 0)
        m_DummyInput.m_Fire++;
    
    m_IsDummySwapping = 1;
}
```

---

## 7. Функция OnSnapInput

### БЫЛО:
```cpp
int CGameClient::OnSnapInput(int *pData, bool Dummy, bool Force)
{
    if(!Dummy)
    {
        return m_Controls.SnapInput(pData);
    }
    
    if(m_aLocalIds[!g_Config.m_ClDummy] < 0)
    {
        return 0;
    }

    if(!g_Config.m_ClDummyHammer)
    {
        // код для обычного dummy input
        // ...
    }
    else
    {
        // код для hammer
        const vec2 Dir = m_LocalCharacterPos - m_aClients[m_aLocalIds[!g_Config.m_ClDummy]].m_Predicted.m_Pos;
        // ...
    }
}
```

### СТАЛО:
```cpp
int CGameClient::OnSnapInput(int *pData, bool Dummy, bool Force)
{
    if(!Dummy)
    {
        return m_Controls.SnapInput(pData);
    }

    // Определяем для какого dummy этот input
    int InputConnection = g_Config.m_ClDummy ^ Dummy;
    int ControlledConnection = g_Config.m_ClDummy;

    // Проверка валидности
    if(m_aLocalIds[ControlledConnection] < 0)
    {
        return 0;
    }

    // Определяем парный dummy
    int TargetConnection;
    if(ControlledConnection <= 1)
    {
        TargetConnection = 1 - ControlledConnection; // 0->1, 1->0
    }
    else
    {
        TargetConnection = 5 - ControlledConnection; // 2->3, 3->2
    }

    // Применяем специальную логику только для парного dummy
    bool ShouldApplySpecialLogic = (InputConnection == TargetConnection);

    if(!g_Config.m_ClDummyHammer || !ShouldApplySpecialLogic)
    {
        // Не отправляем input для dummy из другой пары
        if(InputConnection != TargetConnection)
        {
            return 0;
        }

        // код для обычного dummy input
        // ...
    }
    else
    {
        // код для hammer
        if(m_aLocalIds[ControlledConnection] >= 0 && m_aLocalIds[InputConnection] >= 0)
        {
            const vec2 Dir = m_aClients[m_aLocalIds[ControlledConnection]].m_Predicted.m_Pos - 
                             m_aClients[m_aLocalIds[InputConnection]].m_Predicted.m_Pos;
            m_HammerInput.m_TargetX = (int)Dir.x;
            m_HammerInput.m_TargetY = (int)Dir.y;
        }
        // ...
    }
}
```

**КРИТИЧЕСКИ ВАЖНО**: Заменить ВСЕ использования `!g_Config.m_ClDummy` на `InputConnection` или `TargetConnection`.

---

## 8. Обработка снапшотов при смене dummy

### Добавить в CClient::Update():

```cpp
if(m_LastDummy != (bool)g_Config.m_ClDummy)
{
    GameClient()->InvalidateSnapshot();
    GameClient()->OnDummySwap();

    // НОВЫЙ КОД: Очистка старых снапшотов
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
                m_aapSnapshots[g_Config.m_ClDummy][SNAP_PREV] = m_aSnapshotStorage[g_Config.m_ClDummy].m_pLast ? 
                                                                  m_aSnapshotStorage[g_Config.m_ClDummy].m_pLast->m_pPrev : nullptr;
                
                if(m_aapSnapshots[g_Config.m_ClDummy][SNAP_CURRENT])
                {
                    m_aCurGameTick[g_Config.m_ClDummy] = m_aapSnapshots[g_Config.m_ClDummy][SNAP_CURRENT]->m_Tick;
                }
                if(m_aapSnapshots[g_Config.m_ClDummy][SNAP_PREV])
                {
                    m_aPrevGameTick[g_Config.m_ClDummy] = m_aapSnapshots[g_Config.m_ClDummy][SNAP_PREV]->m_Tick;
                }
            }
        }
    }
}
```

---

## 9. Ограничение обработки снапшотов

### Изменить циклы обработки снапшотов:

**БЫЛО:**
```cpp
while(true)
{
    if(!m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_pNext)
        break;
    int64_t TickStart = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_Tick * time_freq() / GameTickSpeed();
    if(TickStart >= Now)
        break;

    m_aapSnapshots[!g_Config.m_ClDummy][SNAP_PREV] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT];
    m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_pNext;

    m_aCurGameTick[!g_Config.m_ClDummy] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_Tick;
    m_aPrevGameTick[!g_Config.m_ClDummy] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_PREV]->m_Tick;
}
```

**СТАЛО:**
```cpp
int ProcessedSnapshots = 0;
const int MAX_SNAPSHOTS_PER_UPDATE = 10;

while(true)
{
    if(!m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_pNext)
        break;
    int64_t TickStart = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_Tick * time_freq() / GameTickSpeed();
    if(TickStart >= Now)
        break;

    m_aapSnapshots[!g_Config.m_ClDummy][SNAP_PREV] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT];
    m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_pNext;

    m_aCurGameTick[!g_Config.m_ClDummy] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_Tick;
    m_aPrevGameTick[!g_Config.m_ClDummy] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_PREV]->m_Tick;
    
    ProcessedSnapshots++;
    if(ProcessedSnapshots >= MAX_SNAPSHOTS_PER_UPDATE)
    {
        break;
    }
}
```

---

## 10. Config переменные

### В config_variables.h:

**БЫЛО:**
```cpp
MACRO_CONFIG_INT(ClDummy, cl_dummy, 0, 0, 1, CFGFLAG_CLIENT, "0 - main, 1 - dummy")
MACRO_CONFIG_STR(PlayerDummyName, player_dummy_name, 16, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Name of the dummy")
```

**СТАЛО:**
```cpp
MACRO_CONFIG_INT(ClDummy, cl_dummy, 0, 0, 3, CFGFLAG_CLIENT, "0 - main, 1 - dummy1, 2 - dummy2, 3 - dummy3")
MACRO_CONFIG_STR(PlayerDummyName, player_dummy_name, 16, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Name of dummy1")
MACRO_CONFIG_STR(PlayerDummy2Name, player_dummy2_name, 16, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Name of dummy2")
MACRO_CONFIG_STR(PlayerDummy3Name, player_dummy3_name, 16, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Name of dummy3")
```

---

## 11. Общие паттерны замены

### Паттерн 1: !g_Config.m_ClDummy
```cpp
// Старый код с !g_Config.m_ClDummy нужно заменить на:

// В OnSnapInput:
int InputConnection = g_Config.m_ClDummy ^ Dummy;
// Затем использовать InputConnection вместо !g_Config.m_ClDummy

// Пример:
// БЫЛО: m_aLocalIds[!g_Config.m_ClDummy]
// СТАЛО: m_aLocalIds[InputConnection]
```

### Паттерн 2: Итерация по dummy
```cpp
// БЫЛО:
for(int i = 0; i < 2; i++)

// СТАЛО:
for(int i = 0; i < NUM_DUMMIES; i++)

// ИЛИ только по подключенным dummy:
for(int i = 1; i <= 3; i++)
```

### Паттерн 3: Проверка индекса dummy
```cpp
// Для определения пары:
if(g_Config.m_ClDummy <= 1)
{
    // Работа с парой 0 <-> 1
}
else
{
    // Работа с парой 2 <-> 3
}
```

---

## 12. Типичные ошибки и их исправление

### Ошибка 1: Использование bool вместо массива

**Неправильно:**
```cpp
bool m_DummyConnected;
bool m_DummyConnecting;
```

**Правильно:**
```cpp
bool m_aDummyConnected[NUM_DUMMIES] = {false, false, false, false};
bool m_aDummyConnecting[NUM_DUMMIES] = {false, false, false, false};
```

### Ошибка 2: Хардкод индексов

**Неправильно:**
```cpp
if(Conn == 1)  // CONN_DUMMY
{
    m_DummyConnected = true;
}
```

**Правильно:**
```cpp
if(Conn >= CONN_DUMMY && Conn <= CONN_DUMMY3)
{
    int DummyIndex = Conn - CONN_MAIN;
    m_aDummyConnected[DummyIndex] = true;
}
```

### Ошибка 3: Неправильная логика отключения

**Неправильно:**
```cpp
void DummyDisconnect(const char *pReason)
{
    // Отключение всех dummy
    for(int i = 1; i <= 3; i++)
    {
        // ...
    }
}
```

**Правильно:**
```cpp
void DummyDisconnect(const char *pReason)
{
    // Отключение только dummy 1
    if(m_aDummyConnected[1])
    {
        // ...
    }
}

void DummyDisconnectAll(const char *pReason)
{
    // Отключение всех dummy
    for(int i = 1; i <= 3; i++)
    {
        // ...
    }
}
```

---

## 13. Быстрая проверка изменений

Используйте эти команды для поиска мест, требующих изменений:

```bash
# Найти все места с NUM_DUMMIES
grep -rn "NUM_DUMMIES" src/

# Найти старые использования !g_Config.m_ClDummy
grep -rn "!g_Config.m_ClDummy" src/game/client/

# Найти массивы размером 2
grep -rn "\[2\]" src/engine/client/client.h | grep -E "(Dummy|Input|Snapshot)"

# Найти инициализацию массивов с двумя элементами
grep -rn "{0, 0}" src/engine/client/
grep -rn "{false, false}" src/engine/client/
```

---

## Заключение

Эти примеры покрывают наиболее важные изменения кода. Используйте их как референс при интеграции поддержки dummy 2 и 3 в ваш проект. Обратите особое внимание на:

1. Расширение всех массивов до 4 элементов
2. Правильное использование InputConnection вместо !g_Config.m_ClDummy
3. Разделение логики disconnect на DummyDisconnect и DummyDisconnectAll
4. Добавление логики для пар dummy (0<->1 и 2<->3)
5. Очистку снапшотов при смене dummy

Всегда тестируйте каждое изменение отдельно, чтобы убедиться в корректности работы.
