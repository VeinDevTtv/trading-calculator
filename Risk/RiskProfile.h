#ifndef RISK_PROFILE_H
#define RISK_PROFILE_H

#include <string>
#include <memory>

namespace Risk {
    // Risk allocation strategies
    enum class RiskStrategy {
        FIXED,          // Fixed percentage of account
        COMPOUNDING,    // Percentage of current balance
        KELLY_CRITERION // Optimal sizing based on win rate and RR
    };
    
    class RiskProfile {
    public:
        RiskProfile(const std::string& name, 
                   double defaultRisk = 1.0,
                   RiskStrategy strategy = RiskStrategy::FIXED);
        
        virtual ~RiskProfile() = default;
        
        // Getters
        std::string getName() const { return m_name; }
        double getDefaultRisk() const { return m_defaultRisk; }
        RiskStrategy getStrategy() const { return m_strategy; }
        
        // Setters
        void setName(const std::string& name) { m_name = name; }
        void setDefaultRisk(double risk) { m_defaultRisk = risk; }
        void setStrategy(RiskStrategy strategy) { m_strategy = strategy; }
        
        // Calculate risk amount based on profile
        virtual double calculateRiskAmount(double accountBalance, 
                                          double winRate = 0.0, 
                                          double riskRewardRatio = 0.0) const;
        
        // Create predefined profiles
        static std::shared_ptr<RiskProfile> createConservative();
        static std::shared_ptr<RiskProfile> createModerate();
        static std::shared_ptr<RiskProfile> createAggressive();
        
    protected:
        std::string m_name;
        double m_defaultRisk;
        RiskStrategy m_strategy;
    };
    
    // Kelly Criterion specific implementation
    class KellyRiskProfile : public RiskProfile {
    public:
        KellyRiskProfile(const std::string& name, double defaultRisk = 1.0);
        
        double calculateRiskAmount(double accountBalance, 
                                  double winRate, 
                                  double riskRewardRatio) const override;
    };
}

#endif // RISK_PROFILE_H 