// Only included once in keyhook.c
// So all functions are implemented in the header
#pragma once

#include "keyhook.h"

#include "../texc_data/data.h"
#include "../texc_expand/keyboard.h"
#include "../texc_match/keybuffer.h"

#ifdef _WIN32
#include <windows.h>

static DWORD __keyhook_thread;

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
    LRESULT result = CallNextHookEx(NULL, nCode, wParam, lParam);

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

bool keyhook_raw_run() {
    __keyhook_thread = GetCurrentThreadId();

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

    return true;
}

void keyhook_raw_quit() {
    PostThreadMessage(__keyhook_thread, WM_USER + 1, 0, 0);
}

#else

#include <X11/XKBlib.h>
#include <X11/Xlib.h>
#include <X11/Xproto.h>
#include <X11/Xutil.h>
#include <X11/extensions/XTest.h>
#include <X11/extensions/record.h>
#include <X11/keysym.h>

#include "../texc_utils/logger.h"

Display *__keyhook_display;
KeyCode __keyhook_empty_keycode;

XRecordContext __record_context;

KeySym __to_ascii(char (*buffer)[255], xEvent *event) {
    XKeyEvent key_event;
    key_event.display = __keyhook_display;
    key_event.window = event->u.focus.window;
    key_event.root = XDefaultRootWindow(__keyhook_display);
    key_event.subwindow = None;
    key_event.time = event->u.keyButtonPointer.time;
    key_event.x = 1;
    key_event.y = 1;
    key_event.x_root = 1;
    key_event.y_root = 1;
    key_event.same_screen = True;
    key_event.keycode = event->u.u.detail;
    key_event.state = event->u.keyButtonPointer.state;
    key_event.type = event->u.u.type;

    int _;
    // Makes sure XLookupString works if there is MappingNotify event
    KeySym *map =
        XGetKeyboardMapping(__keyhook_display, key_event.keycode, 1, &_);
    XFree(map);

    KeySym sym;
    int res = XLookupString(&key_event, *buffer, 255 - 1, &sym, NULL);

    // Not a valid key character
    if (!res)
        (*buffer)[0] = '\0';

    return sym;
}

void keyhook_callback(XPointer arg, XRecordInterceptData *data) {
    if (data->category != XRecordFromServer) {
        XRecordFreeData(data);
        return;
    }

    xEvent *event = (xEvent *)data->data;
    BYTE type = event->u.u.type;

    if (type == ButtonPress) {
        keyhook_reset();
        XRecordFreeData(data);
        return;
    }

    if (type != KeyPress && type != KeyRelease) {
        XRecordFreeData(data);
        return;
    }

    char buffer[255] = {0};
    KeySym sym = __to_ascii(&buffer, event);

    KeyEvent key_event = {
        .character = (*buffer),
        .keycode = (int)sym,

        .is_keydown = type == KeyPress,
        .is_ctrldown = keyboard_is_pressed(KEYBOARD_CONTROL),
    };

    keyhook_handle_event(key_event);
}

int __get_empty_keycode(Display *dpy) {
    int low, high;
    XDisplayKeycodes(dpy, &low, &high);

    KeySym *syms = NULL;
    int sym_count = 0;
    syms = XGetKeyboardMapping(dpy, low, high - low, &sym_count);

    for (int i = low; i <= high; i++) {
        for (int j = 0; j < sym_count; j++) {
            int symindex = (i - low) * sym_count + j;
            if (syms[symindex] != 0) {
                break;
            } else {
                XFree(syms);
                XFlush(dpy);
                return i;
            }
        }
    }
    return -1;
}

bool keyhook_raw_run() {
    __keyhook_display = XOpenDisplay(NULL);
    int _;

    if (!XkbQueryExtension(__keyhook_display, &_, &_, &_, &_, &_)) {
        LOGGER_ERROR("XKB extension not installed");
        XCloseDisplay(__keyhook_display);
        return false;
    }

    if (!XTestQueryExtension(__keyhook_display, &_, &_, &_, &_)) {
        LOGGER_ERROR("XTest extension not installed");
        XCloseDisplay(__keyhook_display);
        return false;
    }

    if (!XRecordQueryVersion(__keyhook_display, &_, &_)) {
        LOGGER_ERROR("XRecord extension not installed");
        XCloseDisplay(__keyhook_display);
        return false;
    }

    int empty_code = __get_empty_keycode(__keyhook_display);
    if (empty_code == -1) {
        LOGGER_ERROR("(xlib) No empty keycode available!");
        XCloseDisplay(__keyhook_display);
        return false;
    }
    __keyhook_empty_keycode = empty_code;

    XRecordRange *record_range = XRecordAllocRange();
    if (!record_range) {
        LOGGER_ERROR("(xlib) Failed to create XRecordRange");
        XCloseDisplay(__keyhook_display);
        return false;
    }
    record_range->device_events.first = KeyPress;
    record_range->device_events.last = ButtonPress;

    XSynchronize(__keyhook_display, True);
    // Using another display for XRecordEnableContext
    // Allows us to exit from the event loop by using XRecordDisableContext
    Display *data_display = XOpenDisplay(NULL);

    XRecordClientSpec record_client = XRecordAllClients;
    __record_context = XRecordCreateContext(
        __keyhook_display, 0, &record_client, 1, &record_range, 1);

    XFree(record_range);
    XRecordEnableContext(data_display, __record_context, keyhook_callback,
                         NULL);

    XRecordFreeContext(__keyhook_display, __record_context);

    XCloseDisplay(data_display);
    XCloseDisplay(__keyhook_display);

    return true;
}

void keyhook_raw_quit() {
    XRecordDisableContext(__keyhook_display, __record_context);
}

#endif
