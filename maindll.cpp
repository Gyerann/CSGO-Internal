//Headers
#pragma once
#include "offsets.hpp"
#include <iostream>
#include <Windows.h>
#include <bitset>

//Macro
#define M_KEY_DOWN -32768
#define M_KEY_UP 0

//bf_   bitfield
//g_    global
//v3_   vec3
//b     bool
//M_    macro

//Globals
uintptr_t localPlayerPtr;
uintptr_t moduleBaseAddr;
uintptr_t glowObjManAddr;
int localPlayerTeam;

struct vec3
{
    float x, y, z;
};

bool isPlayerMoving(vec3 velocityVector)
{
    float vel = velocityVector.x + velocityVector.y + velocityVector.z;
    if (vel != 0)
        return true;
    else
        return false;
}

//Struct for triggerbot functions and options
struct TriggerBot {

    bool bFlickAttempt{};   //Reaction time to flick shots, switched on by fast mouse movement
    bool bPeek{};           //Reaction time to peeking enemies on corners, switched on by little to no mouse movement
    bool bIsAccurate{};     //Will your next shot be 100% accurate? (standing still + kickback)
    bool bHSOnly{};         //Only trigger on headshots
    bool bScopedOnly{};
    bool bSpray{};          
    int maxSpray{};         //Max bullets before waiting for kickback reset
    int shotDelay = 30;     //Delay to trigger shot

    void Shoot() {
        *(uintptr_t*)(moduleBaseAddr + offsets::dwForceAttack) = 5;
        Sleep(shotDelay);
        *(uintptr_t*)(moduleBaseAddr + offsets::dwForceAttack) = 4;
    }

    bool Check() {

        int targetID = *(uintptr_t*)(localPlayerPtr + offsets::m_iCrosshairId);
        uintptr_t targetEntity = *(uintptr_t*)(moduleBaseAddr + offsets::dwEntityList + ((targetID - 1) * 0x10));
        int targetTeam = *(uintptr_t*)(targetEntity + offsets::m_iTeamNum);
        int targetHealth = *(uintptr_t*)(targetEntity + offsets::m_iHealth);

        if ((targetID != 0)
            &&(targetTeam != localPlayerTeam)
            &&(targetHealth != 0)) {

            return true;
        }
        else {
            return false;
        }
    }

    void Handle() {
        if (Check()) {
            Shoot();
        }
    }
    
} TriggerBot;

//FIX LATER
struct GlowHack {

    uintptr_t actualEntity;
    uintptr_t actualTeam;
    uintptr_t actualRed;
    uintptr_t actualGreen;
    uintptr_t actualBlue;
    uintptr_t actualAlpha;
    uintptr_t glowOccluded;
    uintptr_t glowUnOccluded;
    uintptr_t glowIndex;

    void GetEntities(){
        std::cout << "In GlowHack::GetEntities...\n";
        for (int i = 0; i < 64; i++) {
            actualEntity = *(uintptr_t*)(moduleBaseAddr + offsets::dwEntityList + i * 0x10);

            //Check if entity exists
            if (actualEntity != NULL) {
                //Get addresses
                glowIndex = *(uintptr_t*)(actualEntity + offsets::m_iGlowIndex);
                actualTeam = *(uintptr_t*)(actualEntity + offsets::m_iTeamNum);           
                actualRed = *(uintptr_t*)(glowObjManAddr + (glowIndex * 0x38) + 0x4);
                actualGreen = *(uintptr_t*)(glowObjManAddr + (glowIndex * 0x38) + 0x8);
                actualBlue = *(uintptr_t*)(glowObjManAddr + (glowIndex * 0x38) + 0xc);
                actualAlpha = *(uintptr_t*)(glowObjManAddr + (glowIndex * 0x38) + 0x10);
                glowOccluded = *(uintptr_t*)(glowObjManAddr + (glowIndex * 0x38) + 0x24);
                glowUnOccluded = *(uintptr_t*)(glowObjManAddr + (glowIndex * 0x38) + 0x25);
            }
            else {
                std::cout << "Non-existent entity found, continuing...\n";
                continue;
            }

            //Set glow color based on team
            if (actualTeam == localPlayerTeam) {
                std::cout << "EntTeam == LocalplayerTeam...\n";
                actualRed = 0;
                actualGreen = 0;
                actualBlue = 2;
                std::cout << "Colors set...\n";
                actualAlpha = 1.7f;
                std::cout << "Alpha set...\n";
            }
            else {
                std::cout << "EntTeam != LocalplayerTeam...\n";
                actualRed = 2;
                actualGreen = 0;
                actualBlue = 0;
                std::cout << "Colors set...\n";
                actualAlpha = 1.7f;
                std::cout << "Alpha set...\n";
            }
            //Enable glow
            std::cout << "Trying to enable glowOccluded...\n";
            glowOccluded = true;
            std::cout << "Enabled...\n";
            std::cout << "Trying to enable glowUnOccluded...\n";
            glowUnOccluded = false;
            std::cout << "Enabled...";

    }
    }

} GlowHack;

int main(){
    //Open console
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);  

    //Set up const addresses
    moduleBaseAddr = (uintptr_t)GetModuleHandle(L"client.dll");
    
    //Options
    double flashReduction = 100.f;  //Percentage

    //Switches
    bool bRadar = false;
    bool bBHop = false; 
    bool bFlash = false;
    bool bTrigger = false;
    bool bTriggerHold = false;
    bool bGlowHack = false;

    //Bhop
    std::bitset<32> bf_ForceJump_New{};
    int forceJump_New{};

    //Main thread
    while (true) {
        
        //Read data that can change
        localPlayerPtr = *(uintptr_t*)(moduleBaseAddr + offsets::dwLocalPlayer);
        localPlayerTeam = *(uintptr_t*)(localPlayerPtr + offsets::m_iTeamNum);

        //Eject
        if (GetAsyncKeyState(VK_END) & 1) {
            std::cout << "Dll ejected\n";
            Sleep(100);
            //break;
        }

        //Radar NON FUNCTIONAL
        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            bRadar = !bRadar;
            
            if (bRadar) {
                std::cout << "Radar turned on\n";
            }
            else {
                std::cout << "Radar turned off\n";
            }
        }

        if (bRadar) {

        }

        //Bhop
        if (GetAsyncKeyState(VK_NUMPAD2) & 1) {
            bBHop = !bBHop;            

            if (bBHop) {
                std::cout << "Bhop turned on\n";
            }
            else {
                std::cout << "Bhop turned off\n";
            }
        }    

        if (GetAsyncKeyState(VK_SPACE) 
            && bBHop) {           
            //Variables
            BYTE bf_Flags = *(BYTE*)(localPlayerPtr + offsets::m_fFlags);          
            BYTE bf_ForceJump = *(BYTE*)(moduleBaseAddr + offsets::dwForceJump);
            vec3 v3_localPlayerVelocity = *(vec3*)(localPlayerPtr + offsets::m_vecVelocity);          
            bf_ForceJump_New = bf_ForceJump;

            if (isPlayerMoving(v3_localPlayerVelocity) 
                && (bf_Flags & (1 << 0))) {
                    *(uintptr_t*)(moduleBaseAddr + offsets::dwForceJump) = 6;              
                }     
                     
        }

        //No flash
        if (GetAsyncKeyState(VK_NUMPAD3) & 1) {
            bFlash = !bFlash;

            if (bFlash) {
                std::cout << "Flashbang effect disabled\n";
            }
            else {
                std::cout << "Flashbang effect enabled\n";
            }
        }
        
        if (bFlash && (*(uintptr_t*)(localPlayerPtr + offsets::m_flFlashDuration) != 0)) {
            //modifying alpha would be better, upgrade later
            *(uintptr_t*)(localPlayerPtr + offsets::m_flFlashDuration) = 0;
        }
        
        //Trigger
        //Toggle
        if (GetAsyncKeyState(VK_NUMPAD4) & 1) {
            bTrigger = !bTrigger;

            if (bTrigger) {
                std::cout << "Triggerbot activated\n";
            }
            else {
                std::cout << "Triggerbot deactivated\n";
            }
        }
        //Hold
        if (GetAsyncKeyState(VK_XBUTTON2) == M_KEY_DOWN && !bTriggerHold) {
            bTrigger = true;
            bTriggerHold = true;
        }
        if (GetAsyncKeyState(VK_XBUTTON2) == M_KEY_UP && bTriggerHold) {
            bTrigger = false;
            bTriggerHold = false;
        }

        if (bTrigger) {          
                TriggerBot.Handle();
        }        
        
        //Glow
        if (GetAsyncKeyState(VK_NUMPAD5) & 1) {
            bGlowHack = !bGlowHack;

            if (bGlowHack) {
                glowObjManAddr = *(uintptr_t*)(moduleBaseAddr + offsets::dwGlowObjectManager);
                std::cout << "Glow turned on\n";
            }
            else {
                std::cout << "Glow turned off\n";
            }
        }

        if (bGlowHack) {
            GlowHack.GetEntities();
        }

        Sleep(5); 
    }

    //Cleanup
    FreeConsole();
    //FreeLibraryAndExitThread(hModule, 0);

    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH: {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)main, NULL, 0, nullptr);
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


