#ifndef WORKFLOW_NEW_TRADE_WORKFLOW_H
#define WORKFLOW_NEW_TRADE_WORKFLOW_H

#include <memory>
#include "../SessionManager.h"
#include "../Trade.h"

namespace Workflow {
    /**
     * Handles the entire workflow for creating a new trade
     * @param sessionManager Reference to the current session manager
     */
    void newTradeWorkflow(SessionManager& sessionManager);
}

#endif // WORKFLOW_NEW_TRADE_WORKFLOW_H 