#include <windows.h>
#include <iostream>

typedef int (WINAPI* PrototypeMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
PrototypeMessageBoxW originalMessageBoxW = nullptr;

// මුල් Bytes 12 සුරැකීමට variables
BYTE originalBytes[12] = { 0 };
BYTE jumpInstruction[12] = { 0 };

// අපේ Custom Hook Function එක
int WINAPI HookedMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) {
    std::wcout << L"[LOG] Intercepted API Call in Memory!\n";

    DWORD oldProtect;
    
    // 1. තාවකාලිකව Unhook කිරීම (මුල් Bytes 12 ආපහු ලිවීම)
    VirtualProtect((LPVOID)originalMessageBoxW, 12, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((void*)originalMessageBoxW, originalBytes, 12);
    VirtualProtect((LPVOID)originalMessageBoxW, 12, oldProtect, &oldProtect);

    // 2. දැන් ආරක්ෂිතව මුල් MessageBoxW එක Call කිරීම (Loop වෙන්නේ නැත)
    int result = originalMessageBoxW(hWnd, L"Intercepted successfully without infinite loops!", L"Secured Window", uType);

    // 3. නැවත Hook එක ස්ථාපනය කිරීම (Re-hook)
    VirtualProtect((LPVOID)originalMessageBoxW, 12, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((void*)originalMessageBoxW, jumpInstruction, 12);
    VirtualProtect((LPVOID)originalMessageBoxW, 12, oldProtect, &oldProtect);

    return result;
}

void InstallHook() {
    HMODULE hUser32 = GetModuleHandleA("user32.dll");
    originalMessageBoxW = (PrototypeMessageBoxW)GetProcAddress(hUser32, "MessageBoxW");

    if (!originalMessageBoxW) return;

    DWORD oldProtect;
    VirtualProtect((LPVOID)originalMessageBoxW, 12, PAGE_EXECUTE_READWRITE, &oldProtect);

    // මුල්ම පියවරේදී `MessageBoxW` හි තිබූ සැබෑ Bytes 12 copy කර තබා ගැනීම
    memcpy(originalBytes, (void*)originalMessageBoxW, 12);

    // 64-bit Absolute Jump එක සකස් කිරීම
    jumpInstruction[0] = 0x48; 
    jumpInstruction[1] = 0xB8; // mov rax, address
    UINT_PTR hookAddress = (UINT_PTR)HookedMessageBoxW;
    memcpy(&jumpInstruction[2], &hookAddress, 8);
    jumpInstruction[10] = 0xFF; 
    jumpInstruction[11] = 0xE0; // jmp rax

    // මුල් API එක මතට අපේ Jump එක ලිවීම
    memcpy((void*)originalMessageBoxW, jumpInstruction, 12);

    VirtualProtect((LPVOID)originalMessageBoxW, 12, oldProtect, &oldProtect);
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);

    std::cout << "[+] Calling original MessageBoxW...\n";
    MessageBoxW(NULL, L"Hello World!", L"Normal Box", MB_OK);

    std::cout << "[+] Installing 64-bit memory hook...\n";
    InstallHook();

    std::cout << "[+] Calling MessageBoxW again (Testing interception)...\n";
    MessageBoxW(NULL, L"Hello World!", L"Normal Box", MB_OK);

    return 0;
}
