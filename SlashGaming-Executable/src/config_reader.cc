/**
 * SlashGaming Game Loader
 * Copyright (C) 2018  Mir Drualga
 *
 * This file is part of SlashGaming Game Loader.
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
 *
 *  Additional permissions under GNU Affero General Public License version 3
 *  section 7
 *
 *  If you modify this Program, or any covered work, by linking or combining
 *  it with any program (or a modified version of that program and its
 *  libraries), containing parts covered by the terms of an incompatible
 *  license, the licensors of this Program grant you additional permission
 *  to convey the resulting work.
 */

#include "config_reader.h"

#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

namespace sgexe {
namespace {

const std::filesystem::path kConfigPath = "SlashGaming-Config.json";

constexpr std::string_view kMainEntryKey = "SlashGaming Game Loader";
constexpr std::string_view kMetaDataKey = "!!!Metadata (Do not modify)!!!";

// Note that this signifies the last version where the config formatting and
// entries were updated. These values do not need to change with respect to API
// file version!
constexpr std::string_view kMajorVersionAKey = "Major Version A";
constexpr int kMajorVersionAValue = 1;
constexpr std::string_view kMajorVersionBKey = "Major Version B";
constexpr int kMajorVersionBValue = 0;
constexpr std::string_view kMinorVersionAKey = "Minor Version A";
constexpr int kMinorVersionAValue = 0;
constexpr std::string_view kMinorVersionBKey = "Minor Version B";
constexpr int kMinorVersionBValue = 0;

constexpr std::string_view kInjectDllsKey = "Inject DLLs";

void
CreateDefaultConfig(
    const std::filesystem::path& config_file_path
) {
  std::ofstream file_stream(config_file_path);
  file_stream << "{}" << std::endl;
}

bool
AddMissingEntries(
    const std::filesystem::path& config_path
) {
  nlohmann::json config;
  if (std::ifstream config_stream(config_path);
      config_stream) {
    config_stream >> config;
  } else {
    return false;
  }

  auto& main_entry = config[kMainEntryKey.data()];
  if (!main_entry.is_object()) {
    main_entry = {};
  }

  auto& metadata_entry = main_entry[kMetaDataKey.data()];
  if (!metadata_entry.is_object()) {
    metadata_entry = {};
  }

  // Check that the actual config version is less than or equal to the expected
  // config version. If the actual is larger, then do not add any new entries.
  // If there are any breaking config changes, then the program will most
  // likely crash.
  auto& major_version_a = metadata_entry[kMajorVersionAKey.data()];
  auto& major_version_b = metadata_entry[kMajorVersionBKey.data()];
  auto& minor_version_a = metadata_entry[kMinorVersionAKey.data()];
  auto& minor_version_b = metadata_entry[kMinorVersionBKey.data()];

  bool is_config_version_smaller = false;

  if (!major_version_a.is_number() || major_version_a < kMajorVersionAValue) {
    major_version_a = kMajorVersionAValue;
    major_version_b = 0;
    minor_version_a = 0;
    minor_version_b = 0;

    is_config_version_smaller = true;
  }

  if (!major_version_b.is_number() || major_version_b < kMajorVersionBValue) {
    major_version_b = kMajorVersionBValue;
    minor_version_a = 0;
    minor_version_b = 0;

    is_config_version_smaller = true;
  }

  if (!minor_version_a.is_number() || minor_version_a < kMinorVersionAValue) {
    minor_version_a = kMinorVersionAValue;
    minor_version_b = 0;

    is_config_version_smaller = true;
  }

  if (!minor_version_b.is_number() || minor_version_b < kMinorVersionBValue) {
    minor_version_b = kMinorVersionBValue;

    is_config_version_smaller = true;
  }

  // The user's config matches or is smaller, so do not add defaults.
  if (!is_config_version_smaller) {
    return true;
  }

  if (auto& entry = main_entry[kInjectDllsKey.data()];
      !entry.is_array()) {
    entry = nlohmann::json::array();
  }

  if (std::fstream config_stream(config_path, std::ios::out | std::ios::trunc);
      config_stream) {
    config_stream << std::setw(4) << config << std::endl;
  } else {
    return false;
  }

  return true;
}

} // namespace

std::vector<std::filesystem::path>
GetLibraryPaths(
    void
) {
  if (!std::filesystem::exists(kConfigPath)) {
    CreateDefaultConfig(kConfigPath);
  }

  bool is_missing_entry_added = AddMissingEntries(kConfigPath);
  if (!is_missing_entry_added) {
    return std::vector<std::filesystem::path>();
  }

  nlohmann::json config;
  if (std::ifstream config_stream(kConfigPath);
      config_stream
  ) {
    config_stream >> config;
  } else {
    return std::vector<std::filesystem::path>();
  }

  const auto& dll_list = config[kMainEntryKey.data()][kInjectDllsKey.data()];

  std::vector<std::filesystem::path> library_paths;
  for (const auto& dll : dll_list) {
    if (dll.is_string()) {
      library_paths.push_back(dll.get<std::string>());
    }
  }

  return library_paths;
}

} // namespace sgexe