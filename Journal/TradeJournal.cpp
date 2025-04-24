#include "TradeJournal.h"
#include <fstream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>

namespace Journal {
    TradeJournal::TradeJournal() {}
    
    void TradeJournal::addEntry(const std::string& tradeId, 
                               const std::string& notes,
                               const std::string& setupReasoning,
                               const std::vector<SentimentTag>& sentimentTags) {
        JournalEntry entry;
        entry.tradeId = tradeId;
        entry.notes = notes;
        entry.setupReasoning = setupReasoning;
        entry.sentimentTags = sentimentTags;
        entry.timestamp = std::time(nullptr);
        
        m_entries[tradeId] = entry;
    }
    
    bool TradeJournal::updateEntry(const std::string& tradeId, 
                                 const std::string& notes,
                                 const std::string& setupReasoning) {
        if (!hasEntry(tradeId)) {
            return false;
        }
        
        m_entries[tradeId].notes = notes;
        
        if (!setupReasoning.empty()) {
            m_entries[tradeId].setupReasoning = setupReasoning;
        }
        
        return true;
    }
    
    bool TradeJournal::addSentimentTag(const std::string& tradeId, SentimentTag tag) {
        if (!hasEntry(tradeId)) {
            return false;
        }
        
        // Check if tag already exists
        auto& tags = m_entries[tradeId].sentimentTags;
        if (std::find(tags.begin(), tags.end(), tag) == tags.end()) {
            tags.push_back(tag);
        }
        
        return true;
    }
    
    bool TradeJournal::removeSentimentTag(const std::string& tradeId, SentimentTag tag) {
        if (!hasEntry(tradeId)) {
            return false;
        }
        
        auto& tags = m_entries[tradeId].sentimentTags;
        auto it = std::find(tags.begin(), tags.end(), tag);
        
        if (it != tags.end()) {
            tags.erase(it);
            return true;
        }
        
        return false;
    }
    
    bool TradeJournal::addLessonLearned(const std::string& tradeId, const std::string& lesson) {
        if (!hasEntry(tradeId)) {
            return false;
        }
        
        m_entries[tradeId].lessonLearned = lesson;
        return true;
    }
    
    JournalEntry TradeJournal::getEntry(const std::string& tradeId) const {
        if (hasEntry(tradeId)) {
            return m_entries.at(tradeId);
        }
        
        // Return empty entry if not found
        return JournalEntry();
    }
    
    std::vector<JournalEntry> TradeJournal::getAllEntries() const {
        std::vector<JournalEntry> entries;
        
        for (const auto& pair : m_entries) {
            entries.push_back(pair.second);
        }
        
        sortEntriesByDate(entries);
        return entries;
    }
    
    std::vector<JournalEntry> TradeJournal::getEntriesByTag(SentimentTag tag) const {
        std::vector<JournalEntry> entries;
        
        for (const auto& pair : m_entries) {
            const auto& entry = pair.second;
            
            // Check if this entry has the specified tag
            if (std::find(entry.sentimentTags.begin(), entry.sentimentTags.end(), tag) != entry.sentimentTags.end()) {
                entries.push_back(entry);
            }
        }
        
        sortEntriesByDate(entries);
        return entries;
    }
    
    bool TradeJournal::hasEntry(const std::string& tradeId) const {
        return m_entries.find(tradeId) != m_entries.end();
    }
    
    bool TradeJournal::exportToCSV(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }
        
        // Write header
        file << "TradeID,Timestamp,Notes,Setup Reasoning,Sentiment Tags,Lesson Learned\n";
        
        // Get all entries sorted by date
        auto entries = getAllEntries();
        
        // Write each entry
        for (const auto& entry : entries) {
            file << entry.tradeId << ",";
            
            // Format timestamp
            std::tm* timeInfo = std::localtime(&entry.timestamp);
            file << std::put_time(timeInfo, "%Y-%m-%d %H:%M:%S") << ",";
            
            // Escape special characters in CSV fields
            auto escapeCsv = [](const std::string& str) -> std::string {
                std::string result = str;
                // Replace double quotes with double-double quotes
                size_t pos = 0;
                while ((pos = result.find("\"", pos)) != std::string::npos) {
                    result.replace(pos, 1, "\"\"");
                    pos += 2;
                }
                
                // If contains comma, newline, or quotes, wrap in quotes
                if (result.find(',') != std::string::npos || 
                    result.find('\n') != std::string::npos ||
                    result.find('"') != std::string::npos) {
                    result = "\"" + result + "\"";
                }
                
                return result;
            };
            
            file << escapeCsv(entry.notes) << ",";
            file << escapeCsv(entry.setupReasoning) << ",";
            
            // Format sentiment tags
            std::string tagsStr;
            for (size_t i = 0; i < entry.sentimentTags.size(); ++i) {
                if (i > 0) tagsStr += "; ";
                tagsStr += sentimentTagToString(entry.sentimentTags[i]);
            }
            file << escapeCsv(tagsStr) << ",";
            
            file << escapeCsv(entry.lessonLearned) << "\n";
        }
        
        file.close();
        return true;
    }
    
    std::string TradeJournal::sentimentTagToString(SentimentTag tag) {
        switch (tag) {
            case SentimentTag::NEUTRAL:
                return "NEUTRAL";
            case SentimentTag::FOMO:
                return "FOMO";
            case SentimentTag::REVENGE:
                return "REVENGE";
            case SentimentTag::OVERCONFIDENT:
                return "OVERCONFIDENT";
            case SentimentTag::HESITANT:
                return "HESITANT";
            case SentimentTag::DISCIPLINED:
                return "DISCIPLINED";
            case SentimentTag::IMPULSIVE:
                return "IMPULSIVE";
            case SentimentTag::PATIENT:
                return "PATIENT";
            default:
                return "UNKNOWN";
        }
    }
    
    SentimentTag TradeJournal::stringToSentimentTag(const std::string& tagStr) {
        if (tagStr == "FOMO") return SentimentTag::FOMO;
        if (tagStr == "REVENGE") return SentimentTag::REVENGE;
        if (tagStr == "OVERCONFIDENT") return SentimentTag::OVERCONFIDENT;
        if (tagStr == "HESITANT") return SentimentTag::HESITANT;
        if (tagStr == "DISCIPLINED") return SentimentTag::DISCIPLINED;
        if (tagStr == "IMPULSIVE") return SentimentTag::IMPULSIVE;
        if (tagStr == "PATIENT") return SentimentTag::PATIENT;
        
        return SentimentTag::NEUTRAL;
    }
    
    void TradeJournal::sortEntriesByDate(std::vector<JournalEntry>& entries) const {
        std::sort(entries.begin(), entries.end(), [](const JournalEntry& a, const JournalEntry& b) {
            return a.timestamp > b.timestamp; // Sort by newest first
        });
    }
    
    // JSON export/import methods would be implemented here
    // For brevity, these are placeholders
    bool TradeJournal::exportToJSON(const std::string& filename) const {
        // TODO: Implement JSON export using a JSON library
        return false;
    }
    
    bool TradeJournal::importFromJSON(const std::string& filename) {
        // TODO: Implement JSON import using a JSON library
        return false;
    }
} 