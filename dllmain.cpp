// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "mem.h"
#include <Windows.h>
#include <iostream>

struct gameOffsets
{
    uintptr_t dwLocalPlayer = 0xDBF4CC;
    uintptr_t dwEntityList = 0x4DDB92C;
    uintptr_t dwGlowObjectManager = 0x5324588;
    uintptr_t m_iGlowIndex = 0x10488;
    uintptr_t m_iTeamNum = 0xF4;

}offsets;

struct values
{
    uintptr_t localPlayer;
    uintptr_t moduleBase;
}val;

DWORD WINAPI HackThread(HMODULE hModule)
{
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);

    val.moduleBase = (uintptr_t)GetModuleHandle(L"client.dll");
    val.localPlayer = *(uintptr_t*)(val.moduleBase + offsets.dwLocalPlayer);

    if(val.localPlayer == NULL)
        while(val.localPlayer == NULL)
            val.localPlayer = *(uintptr_t*)(val.moduleBase + offsets.dwLocalPlayer);

    while (!GetAsyncKeyState(VK_END))
    {
        uintptr_t glowObject = *(uintptr_t*)(val.moduleBase + offsets.dwGlowObjectManager);
        int myTeam = *(int*)(val.localPlayer + offsets.m_iTeamNum);

        for (int i = 0; i < 64; i++)
        {
            uintptr_t entity = *(uintptr_t*)(val.moduleBase + offsets.dwEntityList + i * 0x10);
            if(entity != NULL)
            {
                int glowIndex = *(int*)(entity + offsets.m_iGlowIndex);
                int entityTeam = *(int*)(entity + offsets.m_iTeamNum);
                if (myTeam == entityTeam)
                {
                    *(float*)(glowObject + glowIndex * 0x38 + 0x8) = 0.0f;
                    *(float*)(glowObject + glowIndex * 0x38 + 0xC) = 0.0f;
                    *(float*)(glowObject + glowIndex * 0x38 + 0x10) = 2.0f;
                    *(float*)(glowObject + glowIndex * 0x38 + 0x14) = 1.7f;
                }
                else
                {
                    *(float*)(glowObject + glowIndex * 0x38 + 0x8) = 2.0f;
                    *(float*)(glowObject + glowIndex * 0x38 + 0xC) = 0.0f;
                    *(float*)(glowObject + glowIndex * 0x38 + 0x10) = 0.0f;
                    *(float*)(glowObject + glowIndex * 0x38 + 0x14) = 1.7f;
                }
                *(int*)(glowObject + glowIndex * 0x38 + 0x28) = 1;
            }
        }

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

