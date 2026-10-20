#include "deco.hpp"             // IWYU pragma: keep
#include "color_info.hpp"       // IWYU pragma: keep
#include "output.hpp"           // IWYU pragma: keep
#include "sync.hpp"             // IWYU pragma: keep
#include "terminal.hpp"         // IWYU pragma: keep
#include "format.hpp"           // IWYU pragma: keep

#ifdef DECOTERM_ENABLE_STD_FORMAT
#   undef DECOTERM_ENABLE_STD_FORMAT
#endif

#ifdef DECOTERM_ENABLE_STD_PRINT
#   undef DECOTERM_ENABLE_STD_PRINT
#endif
