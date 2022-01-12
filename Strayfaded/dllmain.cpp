#include "include.h"

void* d3d9Device[119];
BYTE EndSceneBytes[7]{ 0 };
tEndScene oEndScene = nullptr;
LPDIRECT3DDEVICE9 pDevice = nullptr;
WNDPROC oWndProc;

void InitImGui(LPDIRECT3DDEVICE9 pDevice)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX9_Init(pDevice);
}

bool init = false;
bool ShowMenu = true;
HRESULT APIENTRY hkEndScene(LPDIRECT3DDEVICE9 o_pDevice)
{
   if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Insert)))
		ShowMenu = !ShowMenu;
    
    if (!pDevice)
        pDevice = o_pDevice;
    
    if (!init)
    {
        InitImGui(pDevice);
        init = true;
    }

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    
    ImGui::Begin("Example DX9 Multihack", &ShowMenu);
    ImGui::Text("DX9 Multihack");
    ImGui::End();

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    return oEndScene(pDevice);
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
        return true;
    return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam)
{
    DWORD wndProcId;
    GetWindowThreadProcessId(handle, &wndProcId);

    if (GetCurrentProcessId() != wndProcId)
        return TRUE;
    window = handle;
    return FALSE;
}
HWND GetProcess()
{
    window = NULL;
    EnumWindows(EnumWindowsCallback, NULL);
    return window;
}

DWORD WINAPI MainThread(HMODULE hMod)
{
    // Hook
    if (GetD3D9Device(d3d9Device, sizeof(d3d9Device)))
    {
        memcpy(EndSceneBytes, (char*)d3d9Device[42], 7);
        oEndScene = (tEndScene)TrampHook((char*)d3d9Device[42], (char*)hkEndScene, 7);
    }
    window = GetProcess();
    oWndProc = (WNDPROC)SetWindowLongPtr(window, -4, (LONG_PTR)WndProc);

    // Loop
    while (!GetAsyncKeyState(UNINJECT_KB))
    {
        Sleep(500); // Always sleep when you're in a "while" loop because you get 20% of cpu usage if you dont
    }

    // Unhook
    Patch((BYTE*)d3d9Device[42], EndSceneBytes, 7);
    FreeLibraryAndExitThread(hMod, 0);
}

BOOL __stdcall StartThread(HMODULE hModule, LPTHREAD_START_ROUTINE StartAddress)
{
    return CloseHandle(CreateThread(0, 0, (LPTHREAD_START_ROUTINE)StartAddress, hModule, 0, 0));
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        StartThread(hModule, (LPTHREAD_START_ROUTINE)MainThread);
    default:
        break;
    }
    return TRUE;
}
