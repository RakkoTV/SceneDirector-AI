/*
 * Window Detector - Detects active windows on the system
 * Copyright (c) 2026 Ramiro Silva
 */

#include "window-detector.hpp"
#include "scene-director-ai.hpp"

#ifdef _WIN32
#include <windows.h>
#include <tlhelp32.h>
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

extern "C" {

bool detect_windows(struct scene_director_data *data)
{
    if (!data) return false;

    data->window_count = 0;

#ifdef _WIN32
    // Windows implementation
    HWND foreground = GetForegroundWindow();

    // Enumerate all windows
    EnumWindows([](HWND hwnd, LPARAM lParam) -> BOOL {
        struct scene_director_data *data = (struct scene_director_data *)lParam;

        if (!IsWindowVisible(hwnd))
            return TRUE;

        char title[256];
        char class_name[256];
        GetWindowTextA(hwnd, title, sizeof(title));
        GetClassNameA(hwnd, class_name, sizeof(class_name));

        if (strlen(title) == 0)
            return TRUE;

        if (data->window_count < MAX_WINDOWS) {
            struct window_info *win = &data->windows[data->window_count];
            strncpy(win->title, title, sizeof(win->title) - 1);
            strncpy(win->class_name, class_name, sizeof(win->class_name) - 1);
            win->is_focused = (hwnd == foreground);
            win->last_seen = os_gettime_ns();
            data->window_count++;
        }

        return TRUE;
    }, (LPARAM)data);

#elif defined(__APPLE__)
    // macOS implementation
    CFArrayRef windows = CGWindowListCopyWindowInfo(
        kCGWindowListOptionOnScreenOnly | kCGWindowListExcludeDesktopElements,
        kCGNullWindowID
    );

    if (windows) {
        CFIndex count = CFArrayGetCount(windows);

        for (CFIndex i = 0; i < count && data->window_count < MAX_WINDOWS; i++) {
            CFDictionaryRef info = (CFDictionaryRef)CFArrayGetValueAtIndex(windows, i);
            CFStringRef title = (CFStringRef)CFDictionaryGetValue(info, kCGWindowName);
            CFStringRef owner = (CFStringRef)CFDictionaryGetValue(info, kCGWindowOwnerName);

            if (title && owner) {
                struct window_info *win = &data->windows[data->window_count];

                char title_buf[256];
                char owner_buf[256];
                CFStringGetCString(title, title_buf, sizeof(title_buf), kCFStringEncodingUTF8);
                CFStringGetCString(owner, owner_buf, sizeof(owner_buf), kCFStringEncodingUTF8);

                strncpy(win->title, title_buf, sizeof(win->title) - 1);
                strncpy(win->class_name, owner_buf, sizeof(win->class_name) - 1);

                CFNumberRef layer = (CFNumberRef)CFDictionaryGetValue(info, kCGWindowLayer);
                int layer_val;
                if (layer && CFNumberGetValue(layer, kCFNumberIntType, &layer_val)) {
                    win->is_focused = (layer_val == 0);
                }

                win->last_seen = os_gettime_ns();
                data->window_count++;
            }
        }

        CFRelease(windows);
    }
#else
    // Linux implementation (X11)
    // Would require Xlib headers
    // For now, return empty
#endif

    return data->window_count > 0;
}

bool match_window_pattern(const char *title, const char *pattern)
{
    if (!title || !pattern)
        return false;

    // Simple wildcard matching
    // * matches any sequence
    // ? matches any single character

    while (*pattern && *title) {
        if (*pattern == '*') {
            pattern++;
            if (!*pattern)
                return true;  // Trailing * matches everything

            const char *tp = title;
            while (*tp) {
                if (match_window_pattern(tp, pattern))
                    return true;
                tp++;
            }
            return false;
        } else if (*pattern == '?' || *pattern == *title) {
            pattern++;
            title++;
        } else {
            return false;
        }
    }

    // Handle trailing *
    while (*pattern == '*')
        pattern++;

    return !*pattern && !*title;
}

}  // extern "C"
