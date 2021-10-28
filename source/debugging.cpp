#include "debugging.h"
#include <3ds.h>

static bool debugging_enabled = false;

bool debugging::should_enable_from_start()
{
#ifdef DEBUGGING
    return true;
#else
    return false;
#endif
}

void debugging::show_error(const char* message)
{
    log(message);

    errorConf err;
    errorInit(&err, errorType::ERROR_TEXT_WORD_WRAP, CFG_Language::CFG_LANGUAGE_EN);
    errorText(&err, message);
    errorDisp(&err);
}

void debugging::enable()
{
    debugging_enabled = true;
    consoleDebugInit(debugDevice_SVC);
}
void debugging::disable()
{
    debugging_enabled = false;
    consoleDebugInit(debugDevice_NULL);
}

bool debugging::enabled()
{
    return debugging_enabled;
}