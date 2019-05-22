#pragma once

#if defined(USESDL)
#include "sdlmain.hpp"
using namespace platform::SDL;
#elif defined(_WIN64) || defined(_WIN32)
/* #include "winmain.hpp" */
/* using namespace platform::windows; */
#include "sdlmain.hpp" // tmp
using namespace platform::SDL;
#elif defined(__linux__)
#include "sdlmain.hpp"
using namespace platform::SDL;
#else
#include "sdlmain.hpp"
using namespace platform::SDL;
#endif
