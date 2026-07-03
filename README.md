# ⚔️ Project: 64-Bit Userland API Hooking Engine
> **A Low-Level Windows Internals Proof-of-Concept for EDR Evasion & Memory Manipulation Research.**

---

### 💻 Technology Stack
| Component | Technology |
| :--- | :--- |
| **Language** | C++ (Standard Template Library) |
| **Architecture** | 64-bit (x86_64 / AMD64) |
| **Compiler** | MinGW-w64 (`x86_64-w64-mingw32-g++`) |
| **Test Environment** | Kali Linux + Wine Emulation Layer |

---

## 🛠️ The Architecture: How It Works

This engine intercepts the native Windows `MessageBoxW` API dynamically inside the process memory.

### 1️⃣ The 12-Byte Absolute Jump Trampoline
Unlike 32-bit architecture which relies on a 5-byte relative jump, 64-bit virtual memory requires an **absolute address** pointer to prevent memory truncation crashes. The engine hot-patches the target function prologue with:

```assembly
mov rax, <64_bit_destination_address>   ; 48 B8 [8-Byte Address]
jmp rax                                 ; FF E0


2️⃣ The Unhook-Execute-Rehook Cycle

To avoid an infinite recursive loop when calling the original function inside our custom proxy function, the engine dynamically orchestrates the following execution pipeline:

[ API Invocations ]
                │
                ▼
    ┌───────────────────────┐
    │  HookedMessageBoxW()  │ ◄─── Intercepted!
    └───────────┬───────────┘
                │
                ▼
    ┌───────────────────────┐
    │  Temporarily UNHOOK   │ ───► Restores original 12 bytes
    └───────────┬───────────┘
                │
                ▼
    ┌───────────────────────┐
    │ Execute Original API  │ ───► Runs native Windows code safely
    └───────────┬───────────┘
                │
                ▼
    ┌───────────────────────┐
    │   RE-HOOK Function    │ ───► Re-injects the 12-byte jump
    └───────────────────────┘


📥 1. Environment Setup

Install the necessary compiler and emulation tools on your Kali Linux instance:

sudo apt update && sudo apt install mingw-w64 wine -y

⚙️ 2. Cross-Compilation

Compile the C++ source file into a static 64-bit Windows PE executable:

x86_64-w64-mingw32-g++ hooking_engine.cpp -o hooking_engine.exe -static

🧪 3. Local Execution

Run the compiled binary directly on Linux using Wine:

wine hooking_engine.exe

📊 Terminal Output Verification

When executed successfully, you will observe the interception trigger after the hook is actively woven into memory:


[+] Calling original MessageBoxW...
[+] Installing 64-bit memory hook...
[+] Calling MessageBoxW again (Testing interception)...
[LOG] Intercepted API Call in Memory!

⚠️ Disclaimer: This repository is strictly for educational, security research, and defensive auditing purposes. All tests were conducted inside controlled sandbox environments.
