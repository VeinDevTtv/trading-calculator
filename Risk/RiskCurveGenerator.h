#ifndef RISK_CURVE_GENERATOR_H
#define RISK_CURVE_GENERATOR_H

#include <vector>
#include <string>
#include <memory>
#include "RiskProfile.h"

namespace Risk {
    struct RiskSimulationParams {
        double initialBalance = 10000.0;
        int numTrades = 100;
        double winRate = 0.55;
        double riskRewardRatio = 2.0;
        double maxRiskPerTrade = 2.0;
        RiskStrategy strategy = RiskStrategy::FIXED;
        bool includeDrawdowns = true;
    };
    
    struct RiskSimulationResult {
        std::vector<double> balanceCurve;
        double finalBalance = 0.0;
        double maxDrawdown = 0.0;
        double maxDrawdownPercent = 0.0;
        int maxConsecutiveLosses = 0;
        double sharpeRatio = 0.0;
        double profitFactor = 0.0;
    };
    
    class RiskCurveGenerator {
    public:
        RiskCurveGenerator();
        ~RiskCurveGenerator() = default;
        
        // Set simulation parameters
        void setSimulationParams(const RiskSimulationParams& params);
        
        // Set custom risk profile
        void setRiskProfile(std::shared_ptr<RiskProfile> profile);
        
        // Run simulation and get results
        RiskSimulationResult generateCurve();
        
        // Export to CSV
        bool exportToCSV(const std::string& filename);
        
        // Get ASCII chart for console display
        std::string getASCIIChart(int width = 70, int height = 15) const;
        
    private:
        RiskSimulationParams m_params;
        RiskSimulationResult m_results;
        std::shared_ptr<RiskProfile> m_riskProfile;
        
        // Helper methods
        double simulateTrade(double balance, double riskPercent, bool isWin);
        void calculateDrawdown(RiskSimulationResult& result);
        void calculateSharpeRatio(RiskSimulationResult& result, const std::vector<double>& returns);
        void calculateProfitFactor(RiskSimulationResult& result, const std::vector<double>& tradeResults);
    };
}

#endif // RISK_CURVE_GENERATOR_H 