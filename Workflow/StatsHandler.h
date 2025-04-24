#ifndef WORKFLOW_STATS_HANDLER_H
#define WORKFLOW_STATS_HANDLER_H

#include "../SessionManager.h"

namespace Workflow {
    /**
     * Calculates and displays session statistics
     * @param sessionManager Reference to the current session manager
     */
    void sessionStatsMode(SessionManager& sessionManager);
}

#endif // WORKFLOW_STATS_HANDLER_H 