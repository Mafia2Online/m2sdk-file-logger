#define ZPL_IMPLEMENTATION
#include "vendor/zpl.h"
#include "m2sdk.h"

#include <thread>
#include <clocale>
#include <unordered_map>

void mod_log(const char* format, ...);

#include "vfs_impl.hpp"
#include "gamemodule.hpp"

zpl_file_t debug_log;
void mod_log(const char* format, ...) {
    va_list ap;
    char message[2048] = { 0 };
    va_start(ap, format);
    vsprintf(message, format, ap);
    va_end(ap);

    // add auto new line at the end
    usize len = zpl_strlen(message);
    if (message[len - 1] != '\n' && len < 2048) {
        message[len + 0] = '\n';
		message[len + 1] = '\0';
        len++;
    }

    message[len - 1] = '\r';
    message[len - 0] = '\n';

    zpl_printf(message);
    zpl_file_write(&debug_log, message, zpl_strlen(message));
}

BOOL APIENTRY DllMain(HMODULE module, DWORD  reason, LPVOID lpReserved) {
    switch (reason) {
        case DLL_PROCESS_ATTACH: {
            AllocConsole();
            AttachConsole(GetCurrentProcessId());
            DisableThreadLibraryCalls(module);

            // Set our locale to the C locale, as Unicode output only functions in this locale
            std::setlocale(LC_ALL, "C");
            SetConsoleOutputCP(CP_UTF8);

            // Relay Input/Output
            FILE* x;
            freopen_s(&x, "CONOUT$", "w", stdout);
            freopen_s(&x, "CONIN$", "r", stdin);

            // Title
            HWND hConsole = GetConsoleWindow();
            SetConsoleTitle("m2-file-logger");

            // Position
            RECT rect;
            GetWindowRect(hConsole, &rect);
            SetWindowPos(hConsole, NULL, 20, 20, 800, 600, 0);

            char temp_path_raw[MAX_PATH] = { '\0' };
            GetModuleFileName(module, temp_path_raw, MAX_PATH);

            auto temp_path = std::string(temp_path_raw);
            auto temp_pos  = temp_path.rfind("\\");

            auto modpath = temp_path.erase(temp_pos, std::string::npos);

            zpl_file_remove((modpath + "\\debug.log").c_str());
            zpl_file_create(&debug_log, (modpath + "\\debug.log").c_str());
            zpl_file_seek(&debug_log, 0);

            mod_log("[info] attaching to thread (%x) ...\n", GetCurrentThreadId());

            vfs_init();
            //vfs_dump_all(true); // dumps all reuqested by the game files into console

            M2::Initialize(mod_install);
        }

        break;
    }

    return TRUE;
};
