#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_CUSTOMCHAT_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_CUSTOMCHAT_H

#include <game/client/component.h>
#include <engine/shared/protocol.h>

class CCustomChat : public CComponent
{
public:
    virtual void OnInit() override;
    virtual void OnRender() override;
    virtual void OnMessage(int MsgType, void *pRawMsg) override;
    virtual int Sizeof() const override { return sizeof(*this); }

private:
    struct CFloatingMessage
    {
        bool m_Active;
        char m_aText[512];
        int64_t m_StartTime;
        int64_t m_EndTime;
    };

    CFloatingMessage m_aFloatingMessages[MAX_CLIENTS];
    void AddMessage(int ClientID, const char *pText);
};

#endif