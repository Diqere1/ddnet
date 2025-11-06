# Руководство по интеграции поддержки Dummy 2 и 3 в DDNet

Это полное руководство по переносу функциональности поддержки 4 dummy (0, 1, 2, 3) из этого репозитория в ваш проект на новой версии DDNet.

## Оглавление
1. [Основные изменения](#основные-изменения)
2. [Изменения в заголовочных файлах](#изменения-в-заголовочных-файлах)
3. [Изменения в engine/client](#изменения-в-engineclient)
4. [Изменения в game/client](#изменения-в-gameclient)
5. [Конфигурационные переменные](#конфигурационные-переменные)
6. [Команды консоли](#команды-консоли)
7. [Логика снапшотов](#логика-снапшотов)
8. [Обработка input](#обработка-input)

---

## Основные изменения

### Ключевые концепции
- **NUM_DUMMIES**: изменено с 2 на 4 (main + 3 dummy)
- **Пары dummy**: (0 ↔ 1) и (2 ↔ 3)
- **Индексы подключений**: CONN_MAIN (0), CONN_DUMMY (1), CONN_DUMMY2 (2), CONN_DUMMY3 (3)

---

## 1. Изменения в заголовочных файлах

### `src/engine/client/enums.h`

**Было:**
```cpp
enum
{
    NUM_DUMMIES = 2,
};
```

**Стало:**
```cpp
enum
{
    NUM_DUMMIES = 4,
};
```

---

### `src/engine/client.h`

#### 1.1. Добавление новых connection constants

**Добавить в enum (около строки 120-128):**
```cpp
enum
{
    CONN_MAIN = 0,
    CONN_DUMMY,      // 1
    CONN_DUMMY2,     // 2
    CONN_DUMMY3,     // 3
    CONN_CONTACT,
    NUM_CONNS,
};
```

#### 1.2. Добавление новых виртуальных методов

**Добавить в public секцию IClient (около строки 168-177):**
```cpp
// dummy
virtual void DummyDisconnect(const char *pReason) = 0;
virtual void DummyDisconnectAll(const char *pReason) = 0;  // НОВОЕ
virtual void DummyConnect() = 0;
virtual void DummyConnect2() = 0;   // НОВОЕ
virtual void DummyConnect3() = 0;   // НОВОЕ
virtual bool DummyConnected() const = 0;
virtual bool DummyConnecting() const = 0;
virtual bool DummyConnectingDelayed() const = 0;
virtual bool DummyAllowed() const = 0;
```

#### 1.3. Расширение массивов состояния

**Изменить protected члены (около строки 90-97):**
```cpp
// quick access to time variables
int m_aPrevGameTick[NUM_DUMMIES] = {0, 0, 0, 0};        // было {0, 0}
int m_aCurGameTick[NUM_DUMMIES] = {0, 0, 0, 0};         // было {0, 0}
float m_aGameIntraTick[NUM_DUMMIES] = {0.0f, 0.0f, 0.0f, 0.0f};  // было {0.0f, 0.0f}
float m_aGameTickTime[NUM_DUMMIES] = {0.0f, 0.0f, 0.0f, 0.0f};   // было {0.0f, 0.0f}
float m_aGameIntraTickSincePrev[NUM_DUMMIES] = {0.0f, 0.0f, 0.0f, 0.0f}; // было {0.0f, 0.0f}

int m_aPredTick[NUM_DUMMIES] = {0, 0, 0, 0};            // было {0, 0}
float m_aPredIntraTick[NUM_DUMMIES] = {0.0f, 0.0f, 0.0f, 0.0f};  // было {0.0f, 0.0f}
```

---

### `src/engine/client/client.h`

#### 2.1. Расширение массивов состояния

**Найти и изменить (около строки 100-120):**
```cpp
uint64_t m_aSnapshotParts[NUM_DUMMIES] = {0, 0, 0, 0};  // было {0, 0}
int m_aAckGameTick[NUM_DUMMIES] = {-1, -1, -1, -1};     // было {-1, -1}
int m_aCurrentRecvTick[NUM_DUMMIES] = {0, 0, 0, 0};     // было {0, 0}
int m_aRconAuthed[NUM_DUMMIES] = {0, 0, 0, 0};          // было {0, 0}

char m_aTimeoutCodes[NUM_DUMMIES][32] = {"", "", "", ""};  // было {"", ""}
bool m_aCodeRunAfterJoin[NUM_DUMMIES] = {false, false, false, false};  // было {false, false}
```

#### 2.2. Массивы для snapshots и inputs

**Изменить:**
```cpp
// Snapshots
CSnapshotStorage m_aSnapshotStorage[NUM_DUMMIES];
CSnapshotStorage::CHolder *m_aapSnapshots[NUM_DUMMIES][NUM_SNAPSHOT_TYPES];
int m_aReceivedSnapshots[NUM_DUMMIES] = {0, 0, 0, 0};   // было {0, 0}
char m_aaSnapshotIncomingData[NUM_DUMMIES][CSnapshot::MAX_SIZE];
int m_aSnapshotIncomingDataSize[NUM_DUMMIES] = {0, 0, 0, 0};  // было {0, 0}

// Inputs
struct m_aInputs[NUM_DUMMIES][200];  // размер массива увеличить
int m_aCurrentInput[NUM_DUMMIES] = {0, 0, 0, 0};  // было {0, 0}

// Dummy state
bool m_aDummySendConnInfo[NUM_DUMMIES] = {false, false, false, false};  // было {false, false}
bool m_aDummyConnecting[NUM_DUMMIES] = {false, false, false, false};    // было {false, false}
bool m_aDummyConnected[NUM_DUMMIES] = {false, false, false, false};     // было {false, false}
```

#### 2.3. Графики для каждого dummy

**Изменить:**
```cpp
CGraph m_aGametimeMarginGraphs[NUM_DUMMIES];  // было [2], теперь [4]
CSmoothTime m_aGameTime[NUM_DUMMIES];         // было [2], теперь [4]
```

#### 2.4. Добавление методов

**Добавить в public секцию:**
```cpp
void DummyDisconnect(const char *pReason) override;
void DummyDisconnectAll(const char *pReason) override;  // НОВОЕ
void DummyConnect() override;
void DummyConnect2() override;   // НОВОЕ
void DummyConnect3() override;   // НОВОЕ
bool DummyConnected() const override;
bool DummyConnecting() const override;
bool DummyConnectingDelayed() const override;
bool DummyAllowed() const override;
```

#### 2.5. Консольные команды

**Добавить в static методы (около строки 418-423):**
```cpp
static void Con_DummyConnect(IConsole::IResult *pResult, void *pUserData);
static void Con_DummyConnect2(IConsole::IResult *pResult, void *pUserData);   // НОВОЕ
static void Con_DummyConnect3(IConsole::IResult *pResult, void *pUserData);   // НОВОЕ
static void Con_DummyDisconnect(IConsole::IResult *pResult, void *pUserData);
static void Con_DummyDisconnectAll(IConsole::IResult *pResult, void *pUserData); // НОВОЕ
static void Con_DummyResetInput(IConsole::IResult *pResult, void *pUserData);
```

#### 2.6. Имена для dummy 2 и 3

**Добавить в private секцию (около строки 270-276):**
```cpp
// For DummyName function
char m_aAutomaticDummyName[MAX_NAME_LENGTH];
char m_aAutomaticDummy2Name[MAX_NAME_LENGTH];   // НОВОЕ
char m_aAutomaticDummy3Name[MAX_NAME_LENGTH];   // НОВОЕ
```

---

## 2. Изменения в `src/engine/client/client.cpp`

### 2.1. Инициализация массивов в конструкторе

**В CClient::CClient() (около строки 86-99):**
```cpp
CClient::CClient() :
    m_DemoPlayer(&m_SnapshotDelta, true, [&]() { UpdateDemoIntraTimers(); }),
    m_InputtimeMarginGraph(128),
    m_aGametimeMarginGraphs{128, 128, 128, 128},  // было {128, 128}
    m_FpsGraph(4096)
{
    m_StateStartTime = time_get();
    for(auto &DemoRecorder : m_aDemoRecorder)
        DemoRecorder = CDemoRecorder(&m_SnapshotDelta);
    m_LastRenderTime = time_get();
    mem_zero(m_aInputs, sizeof(m_aInputs));
    mem_zero(m_aapSnapshots, sizeof(m_aapSnapshots));
    for(auto &SnapshotStorage : m_aSnapshotStorage)
        SnapshotStorage.Init();
    mem_zero(m_aDemorecSnapshotHolders, sizeof(m_aDemorecSnapshotHolders));
}
```

### 2.2. Функции подключения dummy

**Добавить новые функции (около строки 750-870):**

```cpp
void CClient::DummyConnect()
{
    if(!DummyAllowed())
        return;
    
    int DummyToConnect = 1;
    if(m_aDummyConnecting[DummyToConnect] || m_aDummyConnected[DummyToConnect])
        return;
    
    m_aDummyConnecting[DummyToConnect] = true;
    m_LastDummyConnectTime = GlobalTime();
    
    int ConnIndex = CONN_MAIN + DummyToConnect;
    if(m_aNetClient[ConnIndex].State() == NETSTATE_OFFLINE)
    {
        for(int i = 0; i < m_aNetClient[CONN_MAIN].ConnectNum(); ++i)
        {
            m_aNetClient[ConnIndex].Connect(*m_aNetClient[CONN_MAIN].ConnectAddress(i), i);
        }
    }
    
    m_aapSnapshots[DummyToConnect][SNAP_CURRENT] = nullptr;
    m_aapSnapshots[DummyToConnect][SNAP_PREV] = nullptr;
    m_aReceivedSnapshots[DummyToConnect] = 0;
    m_aSnapshotStorage[DummyToConnect].Init();
    m_aGameTime[DummyToConnect].Init(GameTickSpeed());
    m_aGametimeMarginGraphs[DummyToConnect].Init(-150.0f, 150.0f);
}

void CClient::DummyConnect2()
{
    if(!DummyAllowed())
        return;
    
    int DummyToConnect = 2;
    if(m_aDummyConnecting[DummyToConnect] || m_aDummyConnected[DummyToConnect])
        return;
    
    m_aDummyConnecting[DummyToConnect] = true;
    m_LastDummyConnectTime = GlobalTime();
    
    int ConnIndex = CONN_MAIN + DummyToConnect;
    if(m_aNetClient[ConnIndex].State() == NETSTATE_OFFLINE)
    {
        for(int i = 0; i < m_aNetClient[CONN_MAIN].ConnectNum(); ++i)
        {
            m_aNetClient[ConnIndex].Connect(*m_aNetClient[CONN_MAIN].ConnectAddress(i), i);
        }
    }
    
    m_aapSnapshots[DummyToConnect][SNAP_CURRENT] = nullptr;
    m_aapSnapshots[DummyToConnect][SNAP_PREV] = nullptr;
    m_aReceivedSnapshots[DummyToConnect] = 0;
    m_aSnapshotStorage[DummyToConnect].Init();
    m_aGameTime[DummyToConnect].Init(GameTickSpeed());
    m_aGametimeMarginGraphs[DummyToConnect].Init(-150.0f, 150.0f);
}

void CClient::DummyConnect3()
{
    if(!DummyAllowed())
        return;
    
    int DummyToConnect = 3;
    if(m_aDummyConnecting[DummyToConnect] || m_aDummyConnected[DummyToConnect])
        return;
    
    m_aDummyConnecting[DummyToConnect] = true;
    m_LastDummyConnectTime = GlobalTime();
    
    int ConnIndex = CONN_MAIN + DummyToConnect;
    if(m_aNetClient[ConnIndex].State() == NETSTATE_OFFLINE)
    {
        for(int i = 0; i < m_aNetClient[CONN_MAIN].ConnectNum(); ++i)
        {
            m_aNetClient[ConnIndex].Connect(*m_aNetClient[CONN_MAIN].ConnectAddress(i), i);
        }
    }
    
    m_aapSnapshots[DummyToConnect][SNAP_CURRENT] = nullptr;
    m_aapSnapshots[DummyToConnect][SNAP_PREV] = nullptr;
    m_aReceivedSnapshots[DummyToConnect] = 0;
    m_aSnapshotStorage[DummyToConnect].Init();
    m_aGameTime[DummyToConnect].Init(GameTickSpeed());
    m_aGametimeMarginGraphs[DummyToConnect].Init(-150.0f, 150.0f);
}
```

### 2.3. Функции отключения dummy

**Изменить DummyDisconnect и добавить DummyDisconnectAll (около строки 875-920):**

```cpp
void CClient::DummyDisconnect(const char *pReason)
{
    // Disconnect only dummy 1 (CONN_DUMMY)
    if(m_aDummyConnected[1] || m_aDummyConnecting[1])
    {
        m_aNetClient[CONN_DUMMY].Disconnect(pReason);
        m_aRconAuthed[1] = 0;
        m_aapSnapshots[1][SNAP_CURRENT] = nullptr;
        m_aapSnapshots[1][SNAP_PREV] = nullptr;
        m_aReceivedSnapshots[1] = 0;
        m_aDummyConnected[1] = false;
        m_aDummyConnecting[1] = false;
        
        // If we're currently controlling dummy 1, switch back to main
        if(g_Config.m_ClDummy == 1)
        {
            g_Config.m_ClDummy = 0;
        }
        
        GameClient()->OnDummyDisconnect();
    }
}

void CClient::DummyDisconnectAll(const char *pReason)
{
    // Disconnect all dummies (1, 2, 3)
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

### 2.4. Функция DummyConnected

**Изменить (около строки 710-720):**

```cpp
bool CClient::DummyConnected() const
{
    // Проверяем подключение любого dummy
    for(int i = 1; i <= 3; i++)
    {
        if(m_aDummyConnected[i])
            return true;
    }
    return false;
}

bool CClient::DummyConnecting() const
{
    // Проверяем процесс подключения любого dummy
    for(int i = 1; i <= 3; i++)
    {
        if(m_aDummyConnecting[i])
            return true;
    }
    return false;
}
```

### 2.5. Функция DummyName

**Изменить для поддержки всех dummy (около строки 368-390):**

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

### 2.6. Обработка снапшотов при смене dummy

**Добавить в CClient::Update() при смене dummy (около строки 2850-2890):**

```cpp
if(m_LastDummy != (bool)g_Config.m_ClDummy)
{
    // Invalidate references to !m_ClDummy snapshots
    GameClient()->InvalidateSnapshot();
    GameClient()->OnDummySwap();

    // When switching to a dummy, purge old accumulated snapshots to prevent freeze
    // Keep only recent snapshots for smooth transition
    if(m_aapSnapshots[g_Config.m_ClDummy][SNAP_CURRENT])
    {
        // Find the last snapshot in the storage
        CSnapshotStorage::CHolder *pLastHolder = m_aSnapshotStorage[g_Config.m_ClDummy].m_pLast;
        if(pLastHolder)
        {
            // Purge all snapshots except the last few (keep ~5 ticks for smooth interpolation)
            int PurgeUntilTick = pLastHolder->m_Tick - 5;
            if(PurgeUntilTick > 0)
            {
                m_aSnapshotStorage[g_Config.m_ClDummy].PurgeUntil(PurgeUntilTick);
                
                // Reset snapshot pointers to the most recent ones
                m_aapSnapshots[g_Config.m_ClDummy][SNAP_CURRENT] = m_aSnapshotStorage[g_Config.m_ClDummy].m_pLast;
                m_aapSnapshots[g_Config.m_ClDummy][SNAP_PREV] = m_aSnapshotStorage[g_Config.m_ClDummy].m_pLast ? m_aSnapshotStorage[g_Config.m_ClDummy].m_pLast->m_pPrev : nullptr;
                
                // Update ticks to match the new snapshot pointers
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

### 2.7. Ограничение обработки снапшотов

**Добавить лимит на количество обрабатываемых снапшотов в циклах while (около строки 2888-2963):**

```cpp
// Для неактивного dummy
if(m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT])
{
    // switch dummy snapshot
    int64_t Now = m_aGameTime[!g_Config.m_ClDummy].Get(time_get());
    
    // Limit snapshot processing to prevent freeze when switching dummies
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

        // set ticks
        m_aCurGameTick[!g_Config.m_ClDummy] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_CURRENT]->m_Tick;
        m_aPrevGameTick[!g_Config.m_ClDummy] = m_aapSnapshots[!g_Config.m_ClDummy][SNAP_PREV]->m_Tick;
        
        // Prevent processing too many snapshots at once
        ProcessedSnapshots++;
        if(ProcessedSnapshots >= MAX_SNAPSHOTS_PER_UPDATE)
        {
            break;
        }
    }
}

// Для активного dummy - аналогично
if(m_aapSnapshots[g_Config.m_ClDummy][SNAP_CURRENT])
{
    // ... тот же паттерн с лимитом ProcessedSnapshots
}
```

### 2.8. Консольные команды

**Добавить в CClient::RegisterCommands() (около строки 4670-4677):**

```cpp
m_pConsole->Register("dummy_connect", "", CFGFLAG_CLIENT, Con_DummyConnect, this, "Connect dummy");
m_pConsole->Register("dummy_connect2", "", CFGFLAG_CLIENT, Con_DummyConnect2, this, "Connect dummy2");
m_pConsole->Register("dummy_connect3", "", CFGFLAG_CLIENT, Con_DummyConnect3, this, "Connect dummy3");
m_pConsole->Register("dummy_disconnect", "", CFGFLAG_CLIENT, Con_DummyDisconnect, this, "Disconnect dummy 1");
m_pConsole->Register("dummy_disconnect_all", "", CFGFLAG_CLIENT, Con_DummyDisconnectAll, this, "Disconnect all dummies");
m_pConsole->Register("dummy_reset", "", CFGFLAG_CLIENT, Con_DummyResetInput, this, "Reset dummy");
```

**Добавить функции-обработчики (около строки 3700-3720):**

```cpp
void CClient::Con_DummyConnect(IConsole::IResult *pResult, void *pUserData)
{
    CClient *pSelf = (CClient *)pUserData;
    pSelf->DummyConnect();
}

void CClient::Con_DummyConnect2(IConsole::IResult *pResult, void *pUserData)
{
    CClient *pSelf = (CClient *)pUserData;
    pSelf->DummyConnect2();
}

void CClient::Con_DummyConnect3(IConsole::IResult *pResult, void *pUserData)
{
    CClient *pSelf = (CClient *)pUserData;
    pSelf->DummyConnect3();
}

void CClient::Con_DummyDisconnect(IConsole::IResult *pResult, void *pUserData)
{
    CClient *pSelf = (CClient *)pUserData;
    pSelf->DummyDisconnect(nullptr);
}

void CClient::Con_DummyDisconnectAll(IConsole::IResult *pResult, void *pUserData)
{
    CClient *pSelf = (CClient *)pUserData;
    pSelf->DummyDisconnectAll(nullptr);
}
```

### 2.9. Обработка подключения dummy через NETMSG_CON_READY

**Изменить в ProcessServerPacket (около строки 1846-1855):**

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

### 2.10. Disconnect при смене карты

**Изменить в NETMSG_MAP_CHANGE (около строки 1696-1700):**

```cpp
if(DummyConnected() && !m_DummyReconnectOnReload)
{
    DummyDisconnect(nullptr);  // Это теперь отключает только dummy 1
    // Или используйте DummyDisconnectAll(nullptr) если нужно отключить всех
}
```

### 2.11. Переподключение dummy после reload

**Изменить в NETMSG_CON_READY для main (около строки 1838-1845):**

```cpp
else if(Conn == CONN_MAIN && (pPacket->m_Flags & NET_CHUNKFLAG_VITAL) != 0 && Msg == NETMSG_CON_READY)
{
    GameClient()->OnConnected();
    if(m_DummyReconnectOnReload)
    {
        for(int i = 1; i <= 3; i++)  // было i = 1
            m_aDummySendConnInfo[i] = true;
        m_DummyReconnectOnReload = false;
    }
}
```

---

## 3. Изменения в `src/game/client/gameclient.cpp`

### 3.1. Расширение массивов в CGameClient

**В gameclient.h найти и изменить массивы:**

```cpp
class CGameClient : public IGameClient
{
    // ...
    
    int m_aLocalIds[NUM_DUMMIES] = {-1, -1, -1, -1};  // было {-1, -1}
    
    // Другие массивы тоже расширить до NUM_DUMMIES = 4
};
```

### 3.2. Обработка OnDummySwap

**Изменить функцию OnDummySwap (около строки 492-556):**

```cpp
void CGameClient::OnDummySwap()
{
    // Determine the paired connection based on which dummy we're switching to
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

    // Always reset fire state to prevent input replay/stuttering
    m_DummyFire = 0;
    m_HammerInput.m_Fire &= INPUT_STATE_MASK;
    if((m_HammerInput.m_Fire & 1) != 0)
        m_HammerInput.m_Fire++;

    // Reset inputs for dummies from the other pair to prevent unwanted fire
    // When switching between dummy 0/1, reset dummy 2/3, and vice versa
    if(g_Config.m_ClDummy <= 1)
    {
        // Reset dummy 2 and 3
        m_Controls.ResetInput(2);
        m_Controls.ResetInput(3);
    }
    else
    {
        // Reset dummy 0 and 1
        m_Controls.ResetInput(0);
        m_Controls.ResetInput(1);
    }

    // For dummy2 and dummy3, reset inputs instead of copying from another connection
    // to prevent input replay issues
    if(g_Config.m_ClDummy >= 2)
    {
        m_DummyInput = {};
        // Ensure fire is in released state (even value)
        m_DummyInput.m_Fire = 0;
        // Also reset input data for the newly controlled dummy to prevent input replay
        m_Controls.ResetInput(g_Config.m_ClDummy);
    }
    else
    {
        int tmp = m_DummyInput.m_Fire;
        m_DummyInput = m_Controls.m_aInputData[PairedConnection];
        m_Controls.m_aInputData[g_Config.m_ClDummy].m_Fire = tmp;
    }
    
    // Ensure m_DummyInput.m_Fire is in released state (even value)
    m_DummyInput.m_Fire &= INPUT_STATE_MASK;
    if((m_DummyInput.m_Fire & 1) != 0)
        m_DummyInput.m_Fire++;
    
    m_IsDummySwapping = 1;
}
```

### 3.3. Обработка OnSnapInput

**Полностью заменить функцию OnSnapInput (см. файл OnSnapInput_Integration.md):**

Ключевые изменения:
```cpp
int CGameClient::OnSnapInput(int *pData, bool Dummy, bool Force)
{
    if(!Dummy)
    {
        return m_Controls.SnapInput(pData);
    }

    // Determine which connection this input is for
    int InputConnection = g_Config.m_ClDummy ^ Dummy;
    int ControlledConnection = g_Config.m_ClDummy;

    // Check if the controlled connection has a valid character
    if(m_aLocalIds[ControlledConnection] < 0)
    {
        return 0;
    }

    // Determine which connection should receive special commands (like cl_dummyhammer)
    // The pairs are: (0 <-> 1) and (2 <-> 3)
    int TargetConnection;
    if(ControlledConnection <= 1)
    {
        TargetConnection = 1 - ControlledConnection; // 0->1, 1->0
    }
    else
    {
        TargetConnection = 5 - ControlledConnection; // 2->3, 3->2
    }

    // Only apply special logic (like hammer) if this input is for the target connection
    bool ShouldApplySpecialLogic = (InputConnection == TargetConnection);

    if(!g_Config.m_ClDummyHammer || !ShouldApplySpecialLogic)
    {
        // Only send input to the target connection to prevent input replay issues
        if(InputConnection != TargetConnection)
        {
            return 0;
        }

        // ... остальной код
    }
    
    // ... код для hammer и других режимов
}
```

**ВАЖНО**: Заменить все использования `!g_Config.m_ClDummy` на `InputConnection` при обращении к позиции dummy.

### 3.4. Отправка информации о dummy на сервер

**Изменить в OnConnected() или SendInfo (где отправляется CL_STARTINFO):**

```cpp
// При отправке информации определить правильное имя dummy
void CGameClient::SendDummyInfo(int Conn)
{
    int DummyIndex = Conn;
    const char *pName = Client()->DummyName(DummyIndex);
    
    // Отправка CL_STARTINFO с правильным именем
    // ... код отправки
}
```

---

## 4. Конфигурационные переменные

### `src/engine/shared/config_variables.h`

**Изменить:**
```cpp
MACRO_CONFIG_INT(ClDummy, cl_dummy, 0, 0, 3, CFGFLAG_CLIENT, "0 - main, 1 - dummy1, 2 - dummy2, 3 - dummy3")  // было 0, 0, 1
```

**Добавить новые переменные:**
```cpp
MACRO_CONFIG_STR(PlayerDummyName, player_dummy_name, 16, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Name of the dummy")
MACRO_CONFIG_STR(PlayerDummy2Name, player_dummy2_name, 16, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Name of dummy2")  // НОВОЕ
MACRO_CONFIG_STR(PlayerDummy3Name, player_dummy3_name, 16, "", CFGFLAG_CLIENT | CFGFLAG_SAVE, "Name of dummy3")  // НОВОЕ
```

---

## 5. Команды консоли

### Новые команды:
- `dummy_connect` - подключить dummy 1
- `dummy_connect2` - подключить dummy 2
- `dummy_connect3` - подключить dummy 3
- `dummy_disconnect` - отключить только dummy 1
- `dummy_disconnect_all` - отключить все dummy (1, 2, 3)

### Использование:
```
cl_dummy 0  # управление main
cl_dummy 1  # управление dummy 1
cl_dummy 2  # управление dummy 2
cl_dummy 3  # управление dummy 3
```

---

## 6. Проверка работоспособности

### 6.1. Тестовый сценарий

1. Подключитесь к серверу
2. Выполните `dummy_connect` - должен подключиться dummy 1
3. Выполните `dummy_connect2` - должен подключиться dummy 2
4. Выполните `cl_dummy 1` - переключение на dummy 1
5. Выполните `cl_dummy 2` - переключение на dummy 2
6. Проверьте что нет фризов при переключении
7. Проверьте что dummy не делают ненужных ударов при переключении

### 6.2. Возможные проблемы и решения

**Проблема**: Фриз при переключении на dummy 2/3
**Решение**: Убедитесь что добавлена логика purge snapshots и ограничение MAX_SNAPSHOTS_PER_UPDATE

**Проблема**: Dummy делают удары при переключении между парами
**Решение**: Убедитесь что в OnDummySwap добавлен сброс input для другой пары

**Проблема**: Dummy не подключается
**Решение**: Проверьте что сервер поддерживает AllowDummy capability

---

## 7. Важные замечания

### 7.1. Индексация массивов
- **Main**: индекс 0
- **Dummy 1**: индекс 1
- **Dummy 2**: индекс 2
- **Dummy 3**: индекс 3

### 7.2. Пары dummy
- **Пара 1**: Main (0) ↔ Dummy 1 (1)
- **Пара 2**: Dummy 2 (2) ↔ Dummy 3 (3)

### 7.3. Формулы для расчета парных dummy
```cpp
// Для dummy 0 и 1:
int PairedConnection = 1 - ControlledConnection;  // 0->1, 1->0

// Для dummy 2 и 3:
int PairedConnection = 5 - ControlledConnection;  // 2->3, 3->2
```

### 7.4. Расчет InputConnection
```cpp
int InputConnection = g_Config.m_ClDummy ^ Dummy;
// Если Dummy = true (1), то инвертируется младший бит ClDummy
```

---

## 8. Дополнительные файлы

### 8.1. Проверка всех мест использования NUM_DUMMIES

Найдите и обновите все массивы:
```bash
grep -r "NUM_DUMMIES" src/
grep -r "\[2\]" src/ | grep -E "(Dummy|dummy|Input|Snapshot)"
```

### 8.2. Проверка использования !g_Config.m_ClDummy

Замените на правильные индексы:
```bash
grep -r "!g_Config.m_ClDummy" src/game/client/
```

---

## 9. Чек-лист для интеграции

- [ ] Изменен NUM_DUMMIES с 2 на 4
- [ ] Добавлены CONN_DUMMY2 и CONN_DUMMY3
- [ ] Расширены все массивы состояния до [4]
- [ ] Добавлены функции DummyConnect2() и DummyConnect3()
- [ ] Изменена функция DummyDisconnect() для отключения только dummy 1
- [ ] Добавлена функция DummyDisconnectAll()
- [ ] Обновлена функция DummyName() для всех dummy
- [ ] Добавлена логика purge snapshots при смене dummy
- [ ] Добавлено ограничение обработки снапшотов (MAX_SNAPSHOTS_PER_UPDATE)
- [ ] Обновлена функция OnDummySwap() для сброса input другой пары
- [ ] Обновлена функция OnSnapInput() для правильной маршрутизации input
- [ ] Добавлены конфигурационные переменные player_dummy2_name и player_dummy3_name
- [ ] Изменен диапазон cl_dummy с 0-1 на 0-3
- [ ] Добавлены консольные команды и их обработчики
- [ ] Протестирована работа всех dummy
- [ ] Протестировано переключение между dummy без фризов
- [ ] Проверено что нет ненужных ударов при переключении

---

## 10. Полезные ссылки

- [OnSnapInput_Integration.md](./OnSnapInput_Integration.md) - полный код OnSnapInput
- История коммитов в этом репозитории с детальными изменениями

---

## Заключение

Этот гайд покрывает все основные изменения для добавления поддержки 4 dummy в DDNet клиент. Основные изменения касаются:

1. Расширения массивов с 2 до 4 элементов
2. Добавления логики подключения/отключения для dummy 2 и 3
3. Правильной маршрутизации input между парами dummy
4. Оптимизации обработки снапшотов для предотвращения фризов

Следуйте этому руководству последовательно, проверяя каждый шаг, и вы успешно интегрируете поддержку 4 dummy в свой проект.
