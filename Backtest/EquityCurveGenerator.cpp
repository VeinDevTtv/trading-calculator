#include "EquityCurveGenerator.h"
#include <spdlog/spdlog.h>
#include <matplot/matplot.h>
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace Backtest {

EquityCurveGenerator::EquityCurveGenerator(const ChartConfig& config)
    : m_config(config) {
    ensureOutputDirExists();
}

void EquityCurveGenerator::setConfig(const ChartConfig& config) {
    m_config = config;
    ensureOutputDirExists();
}

void EquityCurveGenerator::ensureOutputDirExists() const {
    try {
        std::filesystem::create_directories(m_config.outputDir);
    } catch (const std::exception& e) {
        spdlog::error("Failed to create output directory: {}", e.what());
    }
}

void EquityCurveGenerator::applyChartStyle() {
    auto& f = matplot::gcf();
    
    // Set figure size
    f->size(m_config.width, m_config.height);
    f->resolution(m_config.dpi, m_config.dpi);
    
    // Apply dark mode if requested
    if (m_config.darkMode) {
        f->color("k");
        auto& ax = matplot::gca();
        ax->color("k");
        ax->x_axis().color("w");
        ax->y_axis().color("w");
        ax->title_color("w");
        ax->x_axis().tick_label_color("w");
        ax->y_axis().tick_label_color("w");
        ax->x_axis().label_color("w");
        ax->y_axis().label_color("w");
        ax->legend_text_color("w");
        ax->grid(true);
        ax->grid_color("gray");
    } else {
        auto& ax = matplot::gca();
        ax->grid(true);
    }
}

std::string EquityCurveGenerator::generateEquityCurve(
    const std::string& strategyName, const BacktestResult& result) {
    
    try {
        // Extract equity curve data
        if (result.equityCurve.empty()) {
            spdlog::warn("Empty equity curve data for strategy: {}", strategyName);
            return "";
        }
        
        // Create date indexes for X-axis
        std::vector<double> x(result.equityCurve.size());
        std::iota(x.begin(), x.end(), 0);
        
        // Get Y values (equity values)
        std::vector<double> y(result.equityCurve.size());
        std::transform(result.equityCurve.begin(), result.equityCurve.end(), y.begin(),
                       [](double value) { return value; });
        
        // Clear previous plot
        matplot::cla();
        
        // Create the plot
        matplot::plot(x, y)->line_width(2).color("blue");
        
        // Add horizontal line at initial equity
        matplot::yline(result.equityCurve.front())->line_width(1).color("gray").line_style("--");
        
        // Add title and labels
        matplot::title("Equity Curve: " + strategyName);
        matplot::xlabel("Trade Number");
        matplot::ylabel("Account Equity");
        
        // Add grid and apply styling
        applyChartStyle();
        
        // Add annotations
        std::stringstream ss;
        ss << "Win Rate: " << std::fixed << std::setprecision(2) << (result.winRate * 100) << "% | ";
        ss << "Profit Factor: " << std::fixed << std::setprecision(2) << result.profitFactor << " | ";
        ss << "Max DD: " << std::fixed << std::setprecision(2) << (result.maxDrawdown * 100) << "%";
        
        matplot::text(0.5, 0.02, ss.str())
            ->alignment(matplot::alignment::center)
            ->position(matplot::text::position::bottom);
        
        // Generate filename
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream timestamp;
        timestamp << std::put_time(&tm, "%Y%m%d_%H%M%S");
        
        std::string filename = m_config.outputDir + "/" + strategyName + "_equity_" + 
                               timestamp.str() + "." + m_config.format;
        
        // Save to file
        matplot::save(filename);
        spdlog::info("Generated equity curve image: {}", filename);
        
        return filename;
        
    } catch (const std::exception& e) {
        spdlog::error("Error generating equity curve for {}: {}", strategyName, e.what());
        return "";
    }
}

std::string EquityCurveGenerator::generateDrawdownChart(
    const std::string& strategyName, const BacktestResult& result) {
    
    try {
        // Extract drawdown data
        if (result.drawdowns.empty()) {
            spdlog::warn("Empty drawdown data for strategy: {}", strategyName);
            return "";
        }
        
        // Create date indexes for X-axis
        std::vector<double> x(result.drawdowns.size());
        std::iota(x.begin(), x.end(), 0);
        
        // Get Y values (drawdown percentages)
        std::vector<double> y(result.drawdowns.size());
        std::transform(result.drawdowns.begin(), result.drawdowns.end(), y.begin(),
                       [](double value) { return -value * 100.0; }); // Negate to show as negative values
        
        // Clear previous plot
        matplot::cla();
        
        // Create the plot
        matplot::plot(x, y)->line_width(2).color("red");
        
        // Add title and labels
        matplot::title("Drawdown Chart: " + strategyName);
        matplot::xlabel("Trade Number");
        matplot::ylabel("Drawdown (%)");
        
        // Add annotations
        std::stringstream ss;
        ss << "Max Drawdown: " << std::fixed << std::setprecision(2) << (result.maxDrawdown * 100) << "%";
        
        matplot::text(0.5, 0.95, ss.str())
            ->alignment(matplot::alignment::center);
        
        // Add grid and apply styling
        applyChartStyle();
        
        // Generate filename
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream timestamp;
        timestamp << std::put_time(&tm, "%Y%m%d_%H%M%S");
        
        std::string filename = m_config.outputDir + "/" + strategyName + "_drawdown_" + 
                               timestamp.str() + "." + m_config.format;
        
        // Save to file
        matplot::save(filename);
        spdlog::info("Generated drawdown chart: {}", filename);
        
        return filename;
        
    } catch (const std::exception& e) {
        spdlog::error("Error generating drawdown chart for {}: {}", strategyName, e.what());
        return "";
    }
}

std::string EquityCurveGenerator::generateMonthlyReturnsHeatmap(
    const std::string& strategyName, const BacktestResult& result) {
    
    try {
        // Check if monthly returns data is available
        if (result.monthlyReturns.empty()) {
            spdlog::warn("No monthly returns data for strategy: {}", strategyName);
            return "";
        }
        
        // Initialize months and years arrays
        std::vector<std::string> months = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", 
                                          "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
        
        // Extract years from the data
        std::set<int> yearSet;
        for (const auto& [date, ret] : result.monthlyReturns) {
            yearSet.insert(date.year);
        }
        
        std::vector<int> years(yearSet.begin(), yearSet.end());
        std::sort(years.begin(), years.end());
        
        // Create data matrix for heatmap
        std::vector<std::vector<double>> data(years.size(), std::vector<double>(12, 0.0));
        
        // Fill with NaN to show missing data
        for (auto& row : data) {
            std::fill(row.begin(), row.end(), std::numeric_limits<double>::quiet_NaN());
        }
        
        // Populate the matrix with monthly returns
        for (const auto& [date, ret] : result.monthlyReturns) {
            auto yearIdx = std::find(years.begin(), years.end(), date.year) - years.begin();
            if (date.month >= 1 && date.month <= 12 && yearIdx < years.size()) {
                data[yearIdx][date.month - 1] = ret * 100.0; // Convert to percentage
            }
        }
        
        // Clear previous plot
        matplot::cla();
        
        // Create heatmap
        matplot::heatmap(data);
        
        // Set x and y labels
        std::vector<std::string> ylabels;
        for (int year : years) {
            ylabels.push_back(std::to_string(year));
        }
        
        matplot::yticks(std::vector<double>(years.size()));
        matplot::yticklabels(ylabels);
        
        matplot::xticks(std::vector<double>(12));
        matplot::xticklabels(months);
        
        // Rotate x labels for better readability
        matplot::xtickangle(45);
        
        // Add title
        matplot::title("Monthly Returns (%): " + strategyName);
        
        // Add colorbar
        matplot::colorbar().label("Return (%)");
        
        // Apply styling
        applyChartStyle();
        
        // Generate filename
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream timestamp;
        timestamp << std::put_time(&tm, "%Y%m%d_%H%M%S");
        
        std::string filename = m_config.outputDir + "/" + strategyName + "_monthly_" + 
                               timestamp.str() + "." + m_config.format;
        
        // Save to file
        matplot::save(filename);
        spdlog::info("Generated monthly returns heatmap: {}", filename);
        
        return filename;
        
    } catch (const std::exception& e) {
        spdlog::error("Error generating monthly returns for {}: {}", strategyName, e.what());
        return "";
    }
}

} // namespace Backtest 