#ifndef WORKFLOW_EQUITY_CURVE_RENDERER_H
#define WORKFLOW_EQUITY_CURVE_RENDERER_H

#include <string>
#include "../SessionManager.h"
#include "../Analytics/EquityStats.h"

namespace Workflow {
    /**
     * Displays and exports equity curve information
     */
    class EquityCurveRenderer {
    public:
        EquityCurveRenderer(const SessionManager& sessionManager);
        
        // Display equity curve as ASCII chart
        void displayEquityCurve(int width = 70, int height = 15) const;
        
        // Display extended statistics
        void displayExtendedStats() const;
        
        // Export equity curve to file
        bool exportToCSV(const std::string& filename) const;
        bool exportToPNG(const std::string& filename) const;
        
    private:
        const SessionManager& m_sessionManager;
        
        // Helper methods
        std::string generateASCIIChart(int width, int height) const;
    };
    
    // Function wrapper for backward compatibility
    void displayEquityCurve(const SessionManager& sessionManager);
}

#endif // WORKFLOW_EQUITY_CURVE_RENDERER_H 