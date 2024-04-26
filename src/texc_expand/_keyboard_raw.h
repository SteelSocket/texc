// Only included once in keyboard.c
// So all functions are implemented in the header
#pragma once

#ifdef _WIN32

#include <Windows.h>
#include <winuser.h>

INPUT __win_create_input(int wscan, DWORD flags) {
    INPUT input;

    input.type = INPUT_KEYBOARD;
    input.ki.wVk = 0;
    input.ki.wScan = wscan;
    input.ki.dwFlags = flags;
    input.ki.time = 0;

    return input;
}

int __win_keycode_from_char(char c) { return LOBYTE(VkKeyScan(c)); }

void _keyboard_raw_press(int vk_code) {
    INPUT input =
        __win_create_input(MapVirtualKeyA(vk_code, 0), KEYEVENTF_SCANCODE);
    SendInput(1, &input, sizeof(INPUT));
}

void _keyboard_raw_release(int vk_code) {
    INPUT input = __win_create_input(MapVirtualKeyA(vk_code, 0),
                                     KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP);
    SendInput(1, &input, sizeof(INPUT));
}

void _keyboard_raw_type(char c) {
    INPUT input = __win_create_input(c, KEYEVENTF_UNICODE);
    SendInput(1, &input, sizeof(INPUT));
}

#else
#error "Not implemented"
#endif
