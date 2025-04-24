#ifndef NEW_TRADE_WORKFLOW_H
#define NEW_TRADE_WORKFLOW_H

#include "SessionManager.h"
#include <memory>

namespace Workflow {
    /**
     * Handles the entire workflow for creating a new trade
     * @param sessionManager Reference to the current session manager
     */
    void newTradeWorkflow(SessionManager& sessionManager);
    void configureTradeParameters(std::shared_ptr<Trade> trade);
    void displayTradeResults(std::shared_ptr<Trade> trade);
}

#endif // NEW_TRADE_WORKFLOW_H 