# Интеграция OnSnapInput для 4 dummy (0, 1, 2, 3)

## Ключевые изменения

В новом коде добавлена поддержка 4 dummy с правильным разделением на пары:
- **Пара 1**: dummy 0 ↔ dummy 1
- **Пара 2**: dummy 2 ↔ dummy 3

## Полный код OnSnapInput

```cpp
int CGameClient::OnSnapInput(int *pData, bool Dummy, bool Force)
{
    if(!Dummy)
    {
        return m_Controls.SnapInput(pData);
    }

    // Determine which connection this input is for
    // The loop in SendInput calculates: i = g_Config.m_ClDummy ^ Dummy
    int InputConnection = g_Config.m_ClDummy ^ Dummy;

    // Determine the currently controlled connection
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

    // Учитываем изменение Fire как активность для отправки у дамми
    static int s_LastSentDummyFire = 0;

    if(!g_Config.m_ClDummyHammer || !ShouldApplySpecialLogic)
    {
        // Only send input to the target connection to prevent input replay issues
        // This ensures that when switching between any dummies, old inputs aren't replayed
        if(InputConnection != TargetConnection)
        {
            // Don't send any input to non-target connections
            return 0;
        }

        if(m_DummyFire != 0)
        {
            m_DummyInput.m_Fire = (m_HammerInput.m_Fire + 1) & ~1;
            m_DummyFire = 0;
        }

        const bool MovementOrHook = (m_DummyInput.m_Direction || m_DummyInput.m_Jump || m_DummyInput.m_Hook);
        const bool FireChanged = (m_DummyInput.m_Fire != s_LastSentDummyFire);
        if(!Force && !MovementOrHook && !FireChanged)
        {
            return 0;
        }

        mem_copy(pData, &m_DummyInput, sizeof(m_DummyInput));
        s_LastSentDummyFire = m_DummyInput.m_Fire;
        return sizeof(m_DummyInput);
    }
    else if(g_Config.m_MRXDummyPseudo)
    {
        // Check if target dummy is frozen
        bool IsDummyFrozen = (m_aLocalIds[InputConnection] >= 0) && 
                             (m_aClients[m_aLocalIds[InputConnection]].m_FreezeEnd > 0);
        
        static int s_TargetHoldTicks = 0;
        static vec2 s_HoldAim = vec2(0.f, 0.f);
        static int64_t LastHammerTime = 0; // Время последнего удара молотом
        
        const vec2 PlayerPos = m_LocalCharacterPos;
        vec2 DummyPos = vec2(0, 0);
        if(m_aLocalIds[InputConnection] >= 0)
        {
            DummyPos = m_aClients[m_aLocalIds[InputConnection]].m_Predicted.m_Pos;
        }
        
        int64_t CurrentTime = time_get();
        static int64_t LastToggleTime = 0; // Время последнего переключения
        static bool DirectionEnabled = false; // Состояние включения/выключения
        static int64_t LastHookTime = 0; // Время последнего использования хука
        int64_t HammerCooldown = time_freq() * TuningList()[0].m_HammerHitFireDelay / 1000.0f;
        
        int CurrentWeapon = WEAPON_HAMMER;
        if(m_aLocalIds[InputConnection] >= 0)
        {
            CurrentWeapon = m_aClients[m_aLocalIds[InputConnection]].m_Predicted.m_ActiveWeapon;
        }
        
        // Серверная геометрия удара молотом
        const float R = 28.0f;
        const float CenterOffset = 0.75f * R; // 21
        const float HitRadiusCenters = 1.5f * R; // 42
        vec2 IntendedAimDir = normalize(PlayerPos - DummyPos);
        if(length(IntendedAimDir) < 1e-3f)
            IntendedAimDir = vec2(1.f, 0.f);
        vec2 IntendedProjStart = DummyPos + IntendedAimDir * CenterOffset;
        bool CanHit = (CurrentWeapon == WEAPON_HAMMER) ? (distance(IntendedProjStart, PlayerPos) <= HitRadiusCenters) : true;

        if(IsDummyFrozen)
        {
            if(!g_Config.m_ClDummyHammer)
            {
                if(m_DummyFire != 0)
                {
                    m_DummyInput.m_Fire = (m_HammerInput.m_Fire + 1) & ~1;
                    m_DummyFire = 0;
                }

                const bool MovementOrHook = (m_DummyInput.m_Direction || m_DummyInput.m_Jump || m_DummyInput.m_Hook);
                const bool FireChanged = (m_DummyInput.m_Fire != s_LastSentDummyFire);
                if(!Force && !MovementOrHook && !FireChanged)
                {
                    return 0;
                }

                mem_copy(pData, &m_DummyInput, sizeof(m_DummyInput));
                s_LastSentDummyFire = m_DummyInput.m_Fire;
                return sizeof(m_DummyInput);
            }
            else
            {
                if(m_DummyFire % 25 != 0)
                {
                    m_DummyFire++;
                    return 0;
                }
                m_DummyFire++;

                m_HammerInput.m_Fire = (m_HammerInput.m_Fire + 1) | 1;
                m_HammerInput.m_WantedWeapon = WEAPON_HAMMER + 1;
                if(!g_Config.m_ClDummyRestoreWeapon)
                {
                    m_DummyInput.m_WantedWeapon = WEAPON_HAMMER + 1;
                }

                // Calculate direction towards the controlled character
                if(m_aLocalIds[ControlledConnection] >= 0 && m_aLocalIds[InputConnection] >= 0)
                {
                    const vec2 Dir = m_aClients[m_aLocalIds[ControlledConnection]].m_Predicted.m_Pos - 
                                     m_aClients[m_aLocalIds[InputConnection]].m_Predicted.m_Pos;
                    m_HammerInput.m_TargetX = (int)Dir.x;
                    m_HammerInput.m_TargetY = (int)Dir.y;
                }

                mem_copy(pData, &m_HammerInput, sizeof(m_HammerInput));
                return sizeof(m_HammerInput);
            }
        }

        // Базовая инициализация: избегаем «залипания» направления при переключении mrx_copy
        m_HammerInput.m_Direction = 0;
        m_HammerInput.m_Jump = 0;
        m_HammerInput.m_Hook = 0;
        m_HammerInput.m_Fire = m_DummyInput.m_Fire;
        // По умолчанию восстанавливаем взгляд дамми на исходное место
        m_HammerInput.m_TargetX = m_DummyInput.m_TargetX;
        m_HammerInput.m_TargetY = m_DummyInput.m_TargetY;

        if(s_TargetHoldTicks > 0 && g_Config.m_MRXTargetHit > 0 && !m_DummyInput.m_Hook)
        {
            // Держим прицел на значении тика удара
            m_HammerInput.m_TargetX = (int)s_HoldAim.x;
            m_HammerInput.m_TargetY = (int)s_HoldAim.y;
            --s_TargetHoldTicks;
        }

        // Обнаружение фронта 1->0 у mrx_copy
        static int s_PrevMrxCopy = 0;
        if(s_PrevMrxCopy == 1 && g_Config.m_MRXCopy == 0)
        {
            m_HammerInput.m_Direction = 0;
            m_HammerInput.m_Jump = 0;
            m_HammerInput.m_Hook = 0;
        }
        s_PrevMrxCopy = g_Config.m_MRXCopy;

        if(g_Config.m_MRXCopy == 1)
        {
            // Полное копирование движений/прыжка/хука
            m_HammerInput.m_Direction = m_DummyInput.m_Direction;
            m_HammerInput.m_Jump = m_DummyInput.m_Jump;
            m_HammerInput.m_Hook = m_DummyInput.m_Hook;
            // Гарантируем молот при включённом копировании
            m_HammerInput.m_WantedWeapon = WEAPON_HAMMER + 1;
            m_DummyInput.m_WantedWeapon = WEAPON_HAMMER + 1;
        }
        else if(g_Config.m_MRXCopy_nomove == 1)
        {
            // Только хук/огонь, без переноса движения
            m_HammerInput.m_Hook = m_DummyInput.m_Hook;
            m_HammerInput.m_Fire = m_DummyInput.m_Fire;
            // Гарантируем молот
            m_HammerInput.m_WantedWeapon = WEAPON_HAMMER + 1;
            m_DummyInput.m_WantedWeapon = WEAPON_HAMMER + 1;
        }

        // Логика для cl_dummy_hammer
        if(g_Config.m_ClDummyHammer)
        {
            // Гарантируем выбор молота
            m_HammerInput.m_WantedWeapon = WEAPON_HAMMER + 1;
            m_DummyInput.m_WantedWeapon = WEAPON_HAMMER + 1;

            if(CanHit)
            {
                if(CurrentTime - LastHammerTime > HammerCooldown)
                {
                    // Проверяем, прошло ли достаточно времени с последнего использования хука
                    if(CurrentTime - LastHookTime > time_freq() * 0.01f) // 10 мс
                    {
                        // Направляем молот в сторону игрока
                        if(m_aLocalIds[ControlledConnection] >= 0 && m_aLocalIds[InputConnection] >= 0)
                        {
                            const vec2 Dir = m_aClients[m_aLocalIds[ControlledConnection]].m_Predicted.m_Pos - 
                                             m_aClients[m_aLocalIds[InputConnection]].m_Predicted.m_Pos;
                            m_HammerInput.m_TargetX = (int)Dir.x;
                            m_HammerInput.m_TargetY = (int)Dir.y;
                            s_HoldAim = Dir;
                            s_TargetHoldTicks = g_Config.m_MRXTargetHit;
                        }
                        // Активируем удар молотом
                        m_HammerInput.m_Fire = (m_HammerInput.m_Fire + 1) | 1;
                        LastHammerTime = CurrentTime;
                    }
                }
            }

            if(CurrentTime - LastHammerTime > time_freq() * 0.01f) // 10 мс после удара
            {
                m_HammerInput.m_Hook = m_DummyInput.m_Hook; // Разрешаем использование хука
                if(m_DummyInput.m_Hook) // Если хук активирован
                {
                    // Направляем хук в сторону взгляда игрока
                    vec2 HookDir = normalize(vec2(m_DummyInput.m_TargetX, m_DummyInput.m_TargetY));
                    m_HammerInput.m_TargetX = (int)(HookDir.x * 100.0f);
                    m_HammerInput.m_TargetY = (int)(HookDir.y * 100.0f);
                    LastHookTime = CurrentTime;
                }
            }
            else
            {
                m_HammerInput.m_Hook = 0; // Блокируем хук
            }
        }

        mem_copy(pData, &m_HammerInput, sizeof(m_HammerInput));
        return sizeof(m_HammerInput);
    }
    else
    {
        // Базовая инициализация для обычного cl_dummy_hammer (без MRXDummyPseudo)
        m_HammerInput.m_Direction = 0;
        m_HammerInput.m_Jump = 0;
        m_HammerInput.m_Hook = 0;
        m_HammerInput.m_Fire = m_DummyInput.m_Fire;

        static int s_PrevMrxCopyPlain = 0;
        if(s_PrevMrxCopyPlain == 1 && g_Config.m_MRXCopy == 0)
        {
            m_HammerInput.m_Direction = 0;
            m_HammerInput.m_Jump = 0;
            m_HammerInput.m_Hook = 0;
        }
        s_PrevMrxCopyPlain = g_Config.m_MRXCopy;

        if(m_DummyFire % 25 != 0)
        {
            m_DummyFire++;
            // Если включено копирование (mrx_copy), отправляем m_HammerInput без удара
            if(g_Config.m_MRXCopy == 1)
            {
                m_HammerInput.m_Direction = m_DummyInput.m_Direction;
                m_HammerInput.m_Jump = m_DummyInput.m_Jump;
                m_HammerInput.m_Fire &= ~1; // гарантированно без удара
                mem_copy(pData, &m_HammerInput, sizeof(m_HammerInput));
                return sizeof(m_HammerInput);
            }
            // Режим без переноса движения
            if(g_Config.m_MRXCopy_nomove == 1)
            {
                m_HammerInput.m_Hook = m_DummyInput.m_Hook;
                m_HammerInput.m_Fire = m_DummyInput.m_Fire & ~1;
                mem_copy(pData, &m_HammerInput, sizeof(m_HammerInput));
                return sizeof(m_HammerInput);
            }
            return 0;
        }
        m_DummyFire++;

        m_HammerInput.m_Fire = (m_HammerInput.m_Fire + 1) | 1;
        m_HammerInput.m_WantedWeapon = WEAPON_HAMMER + 1;
        if(!g_Config.m_ClDummyRestoreWeapon)
        {
            m_DummyInput.m_WantedWeapon = WEAPON_HAMMER + 1;
        }

        // Calculate direction towards the controlled character
        if(m_aLocalIds[ControlledConnection] >= 0 && m_aLocalIds[InputConnection] >= 0)
        {
            const vec2 Dir = m_aClients[m_aLocalIds[ControlledConnection]].m_Predicted.m_Pos - 
                             m_aClients[m_aLocalIds[InputConnection]].m_Predicted.m_Pos;
            m_HammerInput.m_TargetX = (int)Dir.x;
            m_HammerInput.m_TargetY = (int)Dir.y;
        }

        mem_copy(pData, &m_HammerInput, sizeof(m_HammerInput));
        return sizeof(m_HammerInput);
    }
}
```

## Ключевые отличия от оригинального кода

### 1. Определение InputConnection и TargetConnection
```cpp
// Определяем для какого dummy этот input
int InputConnection = g_Config.m_ClDummy ^ Dummy;

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
```

### 2. Проверка ShouldApplySpecialLogic
```cpp
// Применяем hammer логику только если input идет для парного dummy
bool ShouldApplySpecialLogic = (InputConnection == TargetConnection);
```

### 3. Фильтрация input для non-target connections
```cpp
// Не отправляем input для dummy из другой пары
if(InputConnection != TargetConnection)
{
    return 0;
}
```

### 4. Использование правильных ID для расчета позиций
Вместо:
```cpp
m_aClients[m_aLocalIds[!g_Config.m_ClDummy]].m_Predicted.m_Pos
```

Используем:
```cpp
m_aClients[m_aLocalIds[InputConnection]].m_Predicted.m_Pos
m_aClients[m_aLocalIds[ControlledConnection]].m_Predicted.m_Pos
```

## Как это работает

### Пример 1: Играем за dummy 0, hammer для dummy 1
- `ControlledConnection = 0`
- `InputConnection = 0 ^ 1 = 1`  
- `TargetConnection = 1 - 0 = 1`
- `ShouldApplySpecialLogic = true` (1 == 1)
- ✅ Hammer логика применяется

### Пример 2: Играем за dummy 0, input для dummy 2
- `ControlledConnection = 0`
- `InputConnection = 0 ^ 1 = 1` (но если это dummy 2...)
- В реальности dummy 2 и 3 не получают input от пары 0/1
- ❌ Input не отправляется

### Пример 3: Играем за dummy 2, hammer для dummy 3
- `ControlledConnection = 2`
- `InputConnection = 2 ^ 1 = 3`
- `TargetConnection = 5 - 2 = 3`
- `ShouldApplySpecialLogic = true` (3 == 3)
- ✅ Hammer логика применяется

## Важные замечания

1. **Проверка на валидность ID**: Всегда проверяйте `m_aLocalIds[Connection] >= 0` перед использованием
2. **Static переменные**: Они общие для всех dummy, поэтому могут быть проблемы при быстром переключении
3. **FreezeEnd проверка**: Используйте `InputConnection` вместо `!g_Config.m_ClDummy`
4. **Направление молота**: Всегда используйте `ControlledConnection` (куда бить) и `InputConnection` (кто бьет)
