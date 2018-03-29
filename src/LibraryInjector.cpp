/**
 * SlashDiablo Game Loader
 * Copyright (C) 2018 Mir Drualga
 *
 *  This file is part of SlashDiablo Game Loader.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LibraryInjector.h"

#include <windows.h>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_set>

const std::unordered_set<std::wstring>& getLibraryPaths() {
    // Define libraries here.
    static const std::unordered_set<std::wstring> libraryPaths = {
        L"BH.dll",
        L"D2HD.dll"
    };
    return libraryPaths;
}

bool injectLibraries(const PROCESS_INFORMATION *pProcessInformation) {
    void* pAllocatedRemoteMemory =
        VirtualAllocEx(pProcessInformation->hProcess, nullptr, MAX_PATH,
            MEM_COMMIT, PAGE_READWRITE);

    if (pAllocatedRemoteMemory == nullptr) {
        return false;
    }

    //WriteProcessMemory();

    if (!VirtualFreeEx(pProcessInformation->hProcess, pAllocatedRemoteMemory,
            MAX_PATH, MEM_DECOMMIT)) {
        std::cout << "Somehow, deallocation failed." << std::endl;
        return false;
    }

    return true;
}

bool loadLibraries() {
    for (const auto& libraryPath : getLibraryPaths()) {
        loadLibrarySafely(libraryPath);
    }
    return true;
}

HMODULE loadLibrarySafely(std::wstring_view libraryPath) {
    HMODULE dllHandle = GetModuleHandleW(libraryPath.data());
    if (dllHandle == nullptr) {
        dllHandle = LoadLibraryW(libraryPath.data());
    }

    return dllHandle;
}