#ifndef SETTINGS_HANDLER_H
#define SETTINGS_HANDLER_H

#include "SessionManager.h"

namespace Workflow {
    /**
     * Handles user configuration settings
     * @param sessionManager Reference to the current session manager
     */
    void configureSettings(SessionManager& sessionManager);
}

#endif // SETTINGS_HANDLER_H 