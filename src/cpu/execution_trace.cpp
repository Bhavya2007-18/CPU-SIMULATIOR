#include "execution_trace.h"

#include <utility>

namespace cpusim {

void ExecutionTrace::add(TraceEntry entry) {
    entries_.push_back(std::move(entry));
}

} // namespace cpusim
