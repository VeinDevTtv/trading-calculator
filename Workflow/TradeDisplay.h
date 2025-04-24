#ifndef WORKFLOW_TRADE_DISPLAY_H
#define WORKFLOW_TRADE_DISPLAY_H

#include <memory>
#include "../Trade.h"

namespace Workflow {
    /**
     * Displays trade results in a formatted way
     * @param trade The trade to display
     */
    void displayTradeResults(std::shared_ptr<Trade> trade);
}

#endif // WORKFLOW_TRADE_DISPLAY_H 