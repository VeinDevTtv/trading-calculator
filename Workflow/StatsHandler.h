#ifndef STATS_HANDLER_H
#define STATS_HANDLER_H

#include "SessionManager.h"

namespace Workflow {
    /**
     * Calculates and displays session statistics
     * @param sessionManager Reference to the current session manager
     */
    void sessionStatsMode(SessionManager& sessionManager);
}

#endif // STATS_HANDLER_H 