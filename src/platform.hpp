#pragma once

#if defined(USESDL)
#include "platform/sdlmain.hpp"
using namespace platform::SDL;
#elif defined(_WIN64) or defined(_WIN32)
/* #include "winmain.hpp" */
/* using namespace platform::windows; */
#include "platform/sdlmain.hpp" // tmp
using namespace platform::SDL;
#elif defined(__linux__)
#include "platform/sdlmain.hpp"
using namespace platform::SDL;
#else
#include "platform/sdlmain.hpp"
using namespace platform::SDL;
#endif
