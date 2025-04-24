#ifndef WORKFLOW_SETTINGS_HANDLER_H
#define WORKFLOW_SETTINGS_HANDLER_H

#include "../SessionManager.h"

namespace Workflow {
    /**
     * Handles user configuration settings
     * @param sessionManager Reference to the current session manager
     */
    void configureSettings(SessionManager& sessionManager);
}

#endif // WORKFLOW_SETTINGS_HANDLER_H 