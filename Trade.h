#ifndef TRADE_H
#define TRADE_H

#include <string>
#include <vector>
#include <ctime>
#include <memory>

// Forward declarations
class TradeCalculator;
struct TradeParameters;
struct TradeResults;

enum class TradeOutcome {
    Pending,
    LossAtSL,
    WinAtTP1,
    WinAtTP2,
    BreakEven
};

enum class InputType {
    Pips,
    Price
};

class Trade {
public:
    Trade();
    ~Trade();

    // Setup methods
    void setAccountBalance(double balance);
    void setRiskPercentage(double riskPercent);
    void setEntryPrice(double price);
    
    // Flexible input methods
    void setStopLoss(double value, InputType type);
    void setTakeProfit(double value, InputType type);
    void setTakeProfit1(double value, InputType type, double percentage = 60.0);
    void setTakeProfit2(double value, InputType type, double percentage = 40.0);
    
    void setInstrumentType(int instrumentTypeIndex);
    void setLotSizeType(int lotSizeTypeIndex);
    void setContractSize(double size);
    
    // Calculation methods
    bool calculate();
    bool calculateWithMultipleTargets();
    
    // Simulation methods
    void simulateOutcome(TradeOutcome outcome);
    double getUpdatedAccountBalance() const;
    
    // Access methods
    TradeParameters getParameters() const;
    TradeResults getResults() const;
    TradeOutcome getOutcome() const;
    std::string getOutcomeAsString() const;
    
    // Utility methods
    bool validate() const;
    std::string getSummary() const;
    bool save(const std::string& filePath, bool append = true) const;
    void reset();
    
    // Time and identification
    std::time_t getTimestamp() const;
    void setTimestamp(std::time_t timestamp);
    std::string getId() const;
    
private:
    std::string m_id;
    std::time_t m_timestamp;
    std::unique_ptr<TradeCalculator> m_calculator;
    std::unique_ptr<TradeParameters> m_params;
    std::unique_ptr<TradeResults> m_results;
    TradeOutcome m_outcome;
    
    // Multiple targets
    double m_tp1Percentage;
    double m_tp2Percentage;
    
    // Input types tracking
    InputType m_slInputType;
    InputType m_tpInputType;
    
    // Helper methods
    double convertPriceToPoints(double entryPrice, double targetPrice) const;
    double convertPipsToPrice(double entryPrice, double pips) const;
    void generateId();
};

#endif // TRADE_H 