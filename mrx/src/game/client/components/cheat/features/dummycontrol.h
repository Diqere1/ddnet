#ifndef GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_DUMMYCONTROL_H
#define GAME_CLIENT_COMPONENTS_CHEAT_FEATURES_DUMMYCONTROL_H

#include <game/client/component.h>
#include <engine/console.h>

class CDummyControl : public CComponent
{
public:
    virtual int Sizeof() const override { return sizeof(*this); }
    virtual void OnConsoleInit() override;
    void MoveLeft();
    void MoveRight();
    void Stop();


private:
    static void ConDummyLeft(IConsole::IResult *pResult, void *pUserData);
    static void ConDummyRight(IConsole::IResult *pResult, void *pUserData);
    static void ConDummyStop(IConsole::IResult *pResult, void *pUserData);
    
};

#endif