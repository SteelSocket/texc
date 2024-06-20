// Only included once in keyhook.c
// So all functions are implemented in the header
#pragma once

#include "keyhook.h"

#include "../texc_data/data.h"

#include "../texc_expand/keyboard.h"
#include "../texc_match/keybuffer.h"

#include "../texc_utils/thread.h"

#ifdef _WIN32
#include <windows.h>

static DWORD __keyhook_thread;
static Mutex *__keyhook_mutex;

char __to_ascii(KBDLLHOOKSTRUCT *kbstruct) {
    BYTE keyboardState[256] = {0};
    DWORD scanCode = kbstruct->scanCode;
    DWORD virtualKeyCode = kbstruct->vkCode;

    if (kbstruct->flags & LLKHF_EXTENDED)
        scanCode |= 0x100;

    HKL keyboardLayout = GetKeyboardLayout(0);

    keyboardState[VK_SHIFT] = GetKeyState(VK_SHIFT) & 0x80;
    keyboardState[VK_CAPITAL] = GetKeyState(VK_CAPITAL) & 0x01;

    UINT mappedScanCode =
        MapVirtualKeyEx(scanCode, MAPVK_VSC_TO_VK_EX, keyboardLayout);

    WCHAR unicodeChar = 0;
    ToUnicodeEx(virtualKeyCode, mappedScanCode, keyboardState, &unicodeChar, 1,
                0, keyboardLayout);

    char typedChar = (char)unicodeChar;

    return typedChar;
}

LRESULT CALLBACK keyhook_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    LRESULT CALLBACK result = CallNextHookEx(NULL, nCode, wParam, lParam);

    if (nCode == HC_ACTION && (wParam == WM_KEYDOWN || wParam == WM_KEYUP)) {
        KBDLLHOOKSTRUCT *kbstruct = (KBDLLHOOKSTRUCT *)lParam;
        DWORD flags = kbstruct->flags;

        DWORD virtual_code = kbstruct->vkCode;
        DWORD scan_code = kbstruct->scanCode;
        char key_char = (kbstruct->vkCode != VK_PACKET)
                            ? __to_ascii(kbstruct)
                            : (char)kbstruct->scanCode;

        KeyEvent event = (KeyEvent){
            .character = key_char,
            .keycode = virtual_code,
            .is_keydown = wParam == WM_KEYDOWN,
            .is_ctrldown = keyboard_is_pressed(VK_CONTROL),
        };

        keyhook_handle_event(event);
    }

    return result;
}

LRESULT CALLBACK mousehook_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION &&
        (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN ||
         wParam == WM_MBUTTONDOWN)) {
        keyhook_reset();
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void keyhook_raw_run() {
    __keyhook_thread = GetCurrentThreadId();
    __keyhook_mutex = mutex_create();

    HHOOK khook = SetWindowsHookEx(WH_KEYBOARD_LL, keyhook_callback, NULL, 0);
    HHOOK mhook = SetWindowsHookEx(WH_MOUSE_LL, mousehook_callback, NULL, 0);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);

        if (msg.message == WM_USER + 1) {
            break;  // Exit the loop when a quit message is received
        }
    }

    UnhookWindowsHookEx(mhook);
    UnhookWindowsHookEx(khook);
    mutex_destroy(__keyhook_mutex);
}

void keyhook_raw_quit() {
    PostThreadMessage(__keyhook_thread, WM_USER + 1, 0, 0);
}

void keyhook_raw_handle_keydown(KeyEvent event) {
    if (event.character == '\b') {
        if (keyhook_try_undo) {
            keyhook_try_undo = false;
            keyhook_reset_undo();
            keybuffer_pop();
        }

        if (keyhook_last_source != NULL && !keyhook_is_expanding) {
            keyhook_try_undo = true;
            return;
        }
        keybuffer_pop();
    } else {
        keybuffer_push(event.character);
        mutex_lock(data.mutex);
        bool replaced = keyhook_check_for_match(event);
        mutex_unlock(data.mutex);

        // Resets keybuffer if enter is pressed 
        // without the cursor being at the end
        if (data.settings.reset_on_enter && !replaced &&
            event.keycode == KEYBOARD_RETURN && keybuffer_cursor) {
            keyhook_reset();
        }
    }

    if (keyhook_last_source && !keyhook_is_expanding) {
        keyhook_reset_undo();
    }
}

void keyhook_raw_handle_keyup(KeyEvent event) {
    if (keyhook_is_expanding) {
        return;
    }

    if (keyhook_try_expand) {
        keyhook_try_expand = false;

        mutex_lock(data.mutex);
        keyhook_expand_matched();
        mutex_unlock(data.mutex);
    }

    if (keyhook_try_undo && !keyhook_is_expanding) {
        keyhook_try_undo = false;
        keyhook_is_expanding = true;

        keybuffer_pop();

        keyboard_backspace(keyhook_expand_len - 1);
        keyboard_nomod_type_string(keyhook_last_source);
        keyhook_reset_undo();

        keyhook_is_expanding = false;

        return;
    }
}

#else
#error "Not implemented"
#endif
