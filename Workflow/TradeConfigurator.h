#ifndef WORKFLOW_TRADE_CONFIGURATOR_H
#define WORKFLOW_TRADE_CONFIGURATOR_H

#include <memory>
#include "../Trade.h"
#include "../Risk/RiskProfile.h"

namespace Workflow {
    /**
     * Configures trade parameters through user interaction
     * @param trade The trade object to configure
     * @param riskProfile Optional custom risk profile to use
     */
    void configureTradeParameters(std::shared_ptr<Trade> trade, std::shared_ptr<Risk::RiskProfile> riskProfile = nullptr);
}

#endif // WORKFLOW_TRADE_CONFIGURATOR_H 