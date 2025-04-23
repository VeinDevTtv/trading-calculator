#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <vector>
#include <memory>
#include <string>
#include "Trade.h"

struct SessionStats {
    double initialBalance = 0.0;
    double currentBalance = 0.0;
    double totalPnL = 0.0;
    double winRate = 0.0;
    int totalTrades = 0;
    int winningTrades = 0;
    int losingTrades = 0;
    int breakEvenTrades = 0;
    double averageRR = 0.0;
    double largestWin = 0.0;
    double largestLoss = 0.0;
    double profitFactor = 0.0;  // Total wins / Total losses
};

class SessionManager {
public:
    SessionManager();
    ~SessionManager();
    
    // Session controls
    void startNewSession(double initialBalance);
    void endSession();
    bool isSessionActive() const;
    double getCurrentBalance() const;
    
    // Trade management
    std::shared_ptr<Trade> createTrade();
    bool addTrade(std::shared_ptr<Trade> trade);
    bool simulateTrade(std::shared_ptr<Trade> trade, TradeOutcome outcome);
    std::shared_ptr<Trade> getTrade(const std::string& id) const;
    std::shared_ptr<Trade> getLastTrade() const;
    std::vector<std::shared_ptr<Trade>> getAllTrades() const;
    
    // Statistics
    SessionStats getSessionStats() const;
    std::string getSessionSummary() const;
    
    // Save/Load
    bool saveSession(const std::string& filename = "");
    bool saveSessionAsJson(const std::string& filename = "");
    bool loadSession(const std::string& filename);
    
    // Configuration
    void setAutoSave(bool autoSave) { m_autoSave = autoSave; }
    bool getAutoSave() const { return m_autoSave; }
    void setSessionFile(const std::string& filename) { m_sessionFile = filename; }
    std::string getSessionFile() const { return m_sessionFile; }
    
private:
    std::vector<std::shared_ptr<Trade>> m_trades;
    bool m_sessionActive;
    double m_initialBalance;
    double m_currentBalance;
    bool m_autoSave;
    std::string m_sessionFile;
    std::string m_sessionId;
    
    // Helper methods
    void updateBalance(const std::shared_ptr<Trade>& trade);
    void generateSessionId();
    void calculateStats(SessionStats& stats) const;
};

#endif // SESSION_MANAGER_H 