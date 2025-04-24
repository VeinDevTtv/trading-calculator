#ifndef JOURNAL_TRADE_JOURNAL_H
#define JOURNAL_TRADE_JOURNAL_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "../Trade.h"

namespace Journal {
    // Sentiment tags for trade psychological factors
    enum class SentimentTag {
        NEUTRAL,
        FOMO,           // Fear of missing out
        REVENGE,        // Revenge trading
        OVERCONFIDENT,  // Overconfidence
        HESITANT,       // Fear/hesitation
        DISCIPLINED,    // Followed plan precisely
        IMPULSIVE,      // Impulsive entry
        PATIENT,        // Waited for setup
    };
    
    // Structure for trade journal entry
    struct JournalEntry {
        std::string tradeId;
        std::string notes;
        std::string setupReasoning;
        std::vector<SentimentTag> sentimentTags;
        std::string lessonLearned;
        std::string marketConditions;
        std::time_t timestamp;
    };
    
    class TradeJournal {
    public:
        TradeJournal();
        ~TradeJournal() = default;
        
        // Add a new journal entry for a trade
        void addEntry(const std::string& tradeId, 
                     const std::string& notes,
                     const std::string& setupReasoning = "",
                     const std::vector<SentimentTag>& sentimentTags = {});
        
        // Update an existing entry
        bool updateEntry(const std::string& tradeId, 
                        const std::string& notes,
                        const std::string& setupReasoning = "");
        
        // Add or update sentiment tags
        bool addSentimentTag(const std::string& tradeId, SentimentTag tag);
        bool removeSentimentTag(const std::string& tradeId, SentimentTag tag);
        
        // Add lesson learned after trade is completed
        bool addLessonLearned(const std::string& tradeId, const std::string& lesson);
        
        // Access and query
        JournalEntry getEntry(const std::string& tradeId) const;
        std::vector<JournalEntry> getAllEntries() const;
        std::vector<JournalEntry> getEntriesByTag(SentimentTag tag) const;
        
        // Check if an entry exists
        bool hasEntry(const std::string& tradeId) const;
        
        // Export entries
        bool exportToJSON(const std::string& filename) const;
        bool exportToCSV(const std::string& filename) const;
        
        // Import entries
        bool importFromJSON(const std::string& filename);
        
        // Get string representation of a sentiment tag
        static std::string sentimentTagToString(SentimentTag tag);
        static SentimentTag stringToSentimentTag(const std::string& tagStr);
        
    private:
        std::map<std::string, JournalEntry> m_entries;
        
        // Helper methods
        void sortEntriesByDate(std::vector<JournalEntry>& entries) const;
    };
}

#endif // JOURNAL_TRADE_JOURNAL_H 