#include "utils.h"

#include <map>
#include <string>

#include "constants.h"

namespace photog {
    photog::Layout get_layout() {
        static std::map<std::string, photog::Layout> layouts =
                {{"planar",      Layout::Planar},
                 {"interleaved", Layout::Interleaved}};

        // LAYOUT is a preprocessor-define set in the build system.
        return layouts.at(LAYOUT);
    }
}
