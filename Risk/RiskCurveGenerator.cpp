#include "RiskCurveGenerator.h"
#include "../Utils.h"
#include <random>
#include <fstream>
#include <cmath>
#include <numeric>
#include <algorithm>

namespace Risk {
    RiskCurveGenerator::RiskCurveGenerator() {
        // Initialize with default risk profile if none is provided
        m_riskProfile = RiskProfile::createModerate();
    }
    
    void RiskCurveGenerator::setSimulationParams(const RiskSimulationParams& params) {
        m_params = params;
    }
    
    void RiskCurveGenerator::setRiskProfile(std::shared_ptr<RiskProfile> profile) {
        if (profile) {
            m_riskProfile = profile;
        }
    }
    
    RiskSimulationResult RiskCurveGenerator::generateCurve() {
        // Reset results
        m_results = RiskSimulationResult();
        m_results.balanceCurve.clear();
        
        // Initial values
        double currentBalance = m_params.initialBalance;
        m_results.balanceCurve.push_back(currentBalance);
        
        // Prepare for simulation
        std::random_device rd;
        std::mt19937 gen(rd());
        std::bernoulli_distribution winDistribution(m_params.winRate);
        
        // Track metrics
        std::vector<double> returns;
        std::vector<double> tradeResults;
        int consecutiveLosses = 0;
        int maxConsecutiveLosses = 0;
        
        // Run simulation
        for (int i = 0; i < m_params.numTrades; ++i) {
            double riskPercent;
            
            // Calculate risk based on strategy and profile
            if (m_riskProfile->getStrategy() == RiskStrategy::KELLY_CRITERION) {
                riskPercent = m_riskProfile->calculateRiskAmount(
                    currentBalance, m_params.winRate, m_params.riskRewardRatio);
            } else if (m_riskProfile->getStrategy() == RiskStrategy::COMPOUNDING) {
                riskPercent = m_riskProfile->getDefaultRisk();
            } else {
                // Fixed risk
                riskPercent = m_riskProfile->getDefaultRisk();
            }
            
            // Cap at max risk
            riskPercent = std::min(riskPercent, m_params.maxRiskPerTrade);
            
            // Simulate trade
            bool isWin = winDistribution(gen);
            double previousBalance = currentBalance;
            
            // Update balance
            currentBalance = simulateTrade(currentBalance, riskPercent, isWin);
            
            // Calculate return
            double tradeReturn = (currentBalance - previousBalance) / previousBalance;
            returns.push_back(tradeReturn);
            
            // Track trade P/L
            double tradePL = currentBalance - previousBalance;
            tradeResults.push_back(tradePL);
            
            // Track consecutive losses
            if (tradePL < 0) {
                consecutiveLosses++;
                maxConsecutiveLosses = std::max(maxConsecutiveLosses, consecutiveLosses);
            } else {
                consecutiveLosses = 0;
            }
            
            // Update balance curve
            m_results.balanceCurve.push_back(currentBalance);
        }
        
        // Calculate final metrics
        m_results.finalBalance = currentBalance;
        m_results.maxConsecutiveLosses = maxConsecutiveLosses;
        
        calculateDrawdown(m_results);
        calculateSharpeRatio(m_results, returns);
        calculateProfitFactor(m_results, tradeResults);
        
        return m_results;
    }
    
    double RiskCurveGenerator::simulateTrade(double balance, double riskPercent, bool isWin) {
        double riskAmount = balance * (riskPercent / 100.0);
        
        if (isWin) {
            return balance + (riskAmount * m_params.riskRewardRatio);
        } else {
            return balance - riskAmount;
        }
    }
    
    void RiskCurveGenerator::calculateDrawdown(RiskSimulationResult& result) {
        double maxBalance = m_params.initialBalance;
        double maxDrawdown = 0.0;
        double drawdownPercent = 0.0;
        
        for (double balance : result.balanceCurve) {
            // Update max balance (high water mark)
            if (balance > maxBalance) {
                maxBalance = balance;
            }
            
            // Calculate current drawdown
            double currentDrawdown = maxBalance - balance;
            double currentDrawdownPercent = (maxBalance > 0) ? (currentDrawdown / maxBalance * 100.0) : 0.0;
            
            // Update max drawdown if current is larger
            if (currentDrawdown > maxDrawdown) {
                maxDrawdown = currentDrawdown;
                drawdownPercent = currentDrawdownPercent;
            }
        }
        
        result.maxDrawdown = maxDrawdown;
        result.maxDrawdownPercent = drawdownPercent;
    }
    
    void RiskCurveGenerator::calculateSharpeRatio(RiskSimulationResult& result, const std::vector<double>& returns) {
        if (returns.empty()) {
            result.sharpeRatio = 0.0;
            return;
        }
        
        // Calculate average return
        double avgReturn = std::accumulate(returns.begin(), returns.end(), 0.0) / returns.size();
        
        // Calculate standard deviation of returns
        double sumSquaredDiff = 0.0;
        for (double ret : returns) {
            double diff = ret - avgReturn;
            sumSquaredDiff += diff * diff;
        }
        
        double stdDev = std::sqrt(sumSquaredDiff / returns.size());
        
        // Calculate annualized Sharpe ratio (approximation assuming daily trades)
        // Sharpe = (Avg Return - Risk Free Rate) / StdDev
        // Using 0 as risk-free rate for simplicity
        result.sharpeRatio = (stdDev > 0) ? (avgReturn / stdDev) * std::sqrt(252.0) : 0.0;
    }
    
    void RiskCurveGenerator::calculateProfitFactor(RiskSimulationResult& result, const std::vector<double>& tradeResults) {
        double totalProfit = 0.0;
        double totalLoss = 0.0;
        
        for (double pl : tradeResults) {
            if (pl > 0) {
                totalProfit += pl;
            } else {
                totalLoss += std::abs(pl);
            }
        }
        
        result.profitFactor = (totalLoss > 0) ? (totalProfit / totalLoss) : 0.0;
    }
    
    bool RiskCurveGenerator::exportToCSV(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        // Write header
        file << "Trade,Balance,Return\n";
        
        // Write data
        for (size_t i = 0; i < m_results.balanceCurve.size(); ++i) {
            double returnPct = 0.0;
            if (i > 0) {
                returnPct = (m_results.balanceCurve[i] - m_results.balanceCurve[i-1]) / 
                           m_results.balanceCurve[i-1] * 100.0;
            }
            
            file << i << "," << m_results.balanceCurve[i] << "," << returnPct << "\n";
        }
        
        file.close();
        return true;
    }
    
    std::string RiskCurveGenerator::getASCIIChart(int width, int height) const {
        if (m_results.balanceCurve.empty()) {
            return "No data to display.";
        }
        
        return Utils::generateASCIIChart(m_results.balanceCurve, width, height);
    }
} 