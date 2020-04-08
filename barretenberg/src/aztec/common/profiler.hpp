#pragma once

#include <string>

namespace profiler {
void start_measurement();
void end_measurement(const std::string& message);
} // namespace profile
