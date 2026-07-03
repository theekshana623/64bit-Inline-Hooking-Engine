#include <windows.h>
#include <iostream>

typedef int (WINAPI* PrototypeMessageBoxW)(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);
PrototypeMessageBoxW originalMessageBoxW = nullptr;

BYTE originalBytes[12] = { 0 };
BYTE jumpInstruction[12] = { 0 };


int WINAPI HookedMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) {
    std::wcout << L"[LOG] Intercepted API Call in Memory!\n";

    DWORD oldProtect;
    
    
    VirtualProtect((LPVOID)originalMessageBoxW, 12, PAGE_EXECUTE_READWRITE, &oldProtect);
    memcpy((void*)originalMessageBoxW, originalBytes, 12);
    VirtualProtect((LPVOID)originalMessageBoxW, 12, oldProtect, &oldProtect);

    
    int result = originalMessageBoxW(hWnd, L"Intercepted successfully without infinite loops!", L"Secured Window", uType);

   
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

   
    memcpy(originalBytes, (void*)originalMessageBoxW, 12);

    
    jumpInstruction[0] = 0x48; 
    jumpInstruction[1] = 0xB8; // mov rax, address
    UINT_PTR hookAddress = (UINT_PTR)HookedMessageBoxW;
    memcpy(&jumpInstruction[2], &hookAddress, 8);
    jumpInstruction[10] = 0xFF; 
    jumpInstruction[11] = 0xE0; // jmp rax

   
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
