#ifndef VIEW_SAVED_TRADES_H
#define VIEW_SAVED_TRADES_H

#include "SessionManager.h"

namespace Workflow {
    /**
     * Displays all saved trades and allows filtering/sorting
     * @param sessionManager Reference to the current session manager
     */
    void viewSavedTrades(SessionManager& sessionManager);
}

#endif // VIEW_SAVED_TRADES_H 