#ifndef WORKFLOW_SIMULATION_HANDLER_H
#define WORKFLOW_SIMULATION_HANDLER_H

#include "../SessionManager.h"

namespace Workflow {
    /**
     * Handles trade simulation functionality
     * @param sessionManager Reference to the current session manager
     */
    void simulationMode(SessionManager& sessionManager);
}

#endif // WORKFLOW_SIMULATION_HANDLER_H 