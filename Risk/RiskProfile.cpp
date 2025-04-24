#include "RiskProfile.h"
#include <algorithm>
#include <cmath>

namespace Risk {
    RiskProfile::RiskProfile(const std::string& name, double defaultRisk, RiskStrategy strategy)
        : m_name(name), m_defaultRisk(defaultRisk), m_strategy(strategy) {}
    
    double RiskProfile::calculateRiskAmount(double accountBalance, double winRate, double riskRewardRatio) const {
        // Default implementation for fixed and compounding strategies
        switch (m_strategy) {
            case RiskStrategy::FIXED:
                return m_defaultRisk;
            
            case RiskStrategy::COMPOUNDING:
                return m_defaultRisk;
                
            case RiskStrategy::KELLY_CRITERION:
                // Should use the derived class implementation
                // If we somehow end up here, provide a safe fallback
                return std::min(m_defaultRisk, 1.0);
                
            default:
                return m_defaultRisk;
        }
    }
    
    std::shared_ptr<RiskProfile> RiskProfile::createConservative() {
        return std::make_shared<RiskProfile>("Conservative", 0.5, RiskStrategy::FIXED);
    }
    
    std::shared_ptr<RiskProfile> RiskProfile::createModerate() {
        return std::make_shared<RiskProfile>("Moderate", 1.0, RiskStrategy::COMPOUNDING);
    }
    
    std::shared_ptr<RiskProfile> RiskProfile::createAggressive() {
        return std::make_shared<KellyRiskProfile>("Aggressive (Kelly)", 2.0);
    }
    
    // Kelly Risk Profile Implementation
    KellyRiskProfile::KellyRiskProfile(const std::string& name, double defaultRisk)
        : RiskProfile(name, defaultRisk, RiskStrategy::KELLY_CRITERION) {}
    
    double KellyRiskProfile::calculateRiskAmount(double accountBalance, double winRate, double riskRewardRatio) const {
        // Kelly Criterion formula: K% = (bp - q) / b
        // where: 
        // b = odds received on the wager (RR)
        // p = probability of winning (winRate)
        // q = probability of losing (1 - winRate)
        
        if (winRate <= 0.0 || winRate >= 1.0 || riskRewardRatio <= 0.0) {
            return m_defaultRisk; // Return default if params invalid
        }
        
        double kellyPercentage = (winRate * riskRewardRatio - (1.0 - winRate)) / riskRewardRatio;
        
        // Bound by half-Kelly for safety and limit to max 5%
        kellyPercentage = kellyPercentage * 0.5; 
        
        // Cap at user-defined max and don't allow negative values
        return std::max(0.0, std::min(kellyPercentage * 100.0, m_defaultRisk));
    }
} 