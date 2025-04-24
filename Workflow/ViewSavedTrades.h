#ifndef WORKFLOW_VIEW_SAVED_TRADES_H
#define WORKFLOW_VIEW_SAVED_TRADES_H

#include "../SessionManager.h"

namespace Workflow {
    /**
     * Displays all saved trades and allows filtering/sorting
     * @param sessionManager Reference to the current session manager
     */
    void viewSavedTrades(SessionManager& sessionManager);
}

#endif // WORKFLOW_VIEW_SAVED_TRADES_H 