// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "mem.h"
#include <Windows.h>
#include <iostream>
#include "csgo.hpp"

using namespace hazedumper::netvars;
using namespace hazedumper::signatures;

struct values
{
    uintptr_t localPlayer;
    uintptr_t moduleBase;
    uintptr_t glowObject;
    uintptr_t engine;
}val;

struct GlowStruct
{
    char pad_0000[8]; //0x0000
    float red; //0x0008
    float green; //0x000C
    float blue; //0x0010
    float alpha; //0x0014
    char pad_0018[16]; //0x0018
    bool renderWhenOccluded; //0x0028
    bool renderWhenUnOccluded; //0x0029
    char pad_002A[14]; //0x002A
};

struct ClrRender
{
    BYTE red, green, blue;
};

ClrRender ClrTeam;
ClrRender ClrEnemy;

GlowStruct SetGlowColor(GlowStruct Glow, uintptr_t entity)
{
    bool defusing = *(bool*)(entity + m_bIsDefusing);
    if (defusing)
    {
        Glow.red = 1.0f;
        Glow.green = 1.0f;
        Glow.blue = 1.0f;
        Glow.alpha = 1.0f;
    }
    else
    { 
        int health = *(int*)(entity + m_iHealth);
        Glow.red = health * - 0.01 + 1;
        Glow.green = health * 0.01;
        Glow.alpha = 1.0f;
    }
    Glow.renderWhenOccluded = true;
    Glow.renderWhenUnOccluded = false;
    return Glow;
}

void SetTeamGlow(uintptr_t entity, int glowIndex)
{
    GlowStruct TGlow;
    TGlow = *(GlowStruct*)(val.glowObject + (glowIndex * 0x38));
    TGlow.blue = 1.0f;
    TGlow.alpha = 1.0f;
    TGlow.renderWhenOccluded = true;
    TGlow.renderWhenUnOccluded = false;
    *(GlowStruct*)(val.glowObject + (glowIndex * 0x38)) = TGlow;
}

void SetEnemyGlow(uintptr_t entity, int glowIndex)
{
    GlowStruct EGlow;
    EGlow = *(GlowStruct*)(val.glowObject + (glowIndex * 0x38));
    EGlow = SetGlowColor(EGlow, entity);
    *(GlowStruct*)(val.glowObject + (glowIndex * 0x38)) = EGlow;
}

void HandleGlow()
{
    val.glowObject = *(uintptr_t*)(val.moduleBase + dwGlowObjectManager);
    int myTeam = *(int*)(val.localPlayer + m_iTeamNum);

    for (int i = 0; i < 64; i++)
    {
        uintptr_t entity = *(uintptr_t*)(val.moduleBase + dwEntityList + i * 0x10);
        if (entity != NULL)
        {
            int glowIndex = *(int*)(entity + m_iGlowIndex);
            int entityTeam = *(int*)(entity + m_iTeamNum);
            if (myTeam == entityTeam)
            { 
                *(ClrRender*)(entity + m_clrRender) = ClrTeam;
                SetTeamGlow(entity, glowIndex);
            }
            else
            {
                *(ClrRender*)(entity + m_clrRender) = ClrEnemy;
                SetEnemyGlow(entity, glowIndex);
            }
        }
    }
}

void SetBrightness()
{
    ClrTeam.red = 0;
    ClrTeam.green = 0;
    ClrTeam.blue = 255;

    ClrEnemy.red = 255;
    ClrEnemy.green = 0;
    ClrEnemy.blue = 0;

    float birghtness = 5.0f;
    int ptr = (int)(val.engine + model_ambient_min);
    *(int*)(val.engine + model_ambient_min) = *(int*)&birghtness ^ ptr;
}

DWORD WINAPI HackThread(HMODULE hModule)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    val.moduleBase = (uintptr_t)GetModuleHandle(L"client.dll");
    val.engine = (uintptr_t)GetModuleHandle(L"engine.dll");
    val.localPlayer = *(uintptr_t*)(val.moduleBase + dwLocalPlayer);

    if(val.localPlayer == NULL)
        while(val.localPlayer == NULL)
            val.localPlayer = *(uintptr_t*)(val.moduleBase + dwLocalPlayer);

    SetBrightness();

    while (!GetAsyncKeyState(VK_END))
    {
        HandleGlow();

        Sleep(1);
    }

    fclose(f);
    FreeConsole();
    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HackThread, hModule, 0, 0);
        if (hThread) CloseHandle(hThread);
    }
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

