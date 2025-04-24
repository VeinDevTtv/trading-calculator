cmake_minimum_required(VERSION 3.14)
project(TradingCalculator VERSION 1.0.0)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_TESTS "Build test suite" ON)
option(USE_MATPLOTPP "Use MatPlot++ for plotting" ON)
option(USE_CAIRO "Use Cairo for plotting" OFF)

# Find required packages
find_package(nlohmann_json REQUIRED)
if(USE_MATPLOTPP)
    find_package(MatPlot++ REQUIRED)
endif()
if(USE_CAIRO)
    find_package(Cairo REQUIRED)
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific settings
    set(CMAKE_WIN32_EXECUTABLE FALSE)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE")
elseif(APPLE)
    # macOS-specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
else()
    # Linux-specific settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
    # Link against stdc++fs for std::filesystem
    target_link_libraries(TradingCalculator PRIVATE stdc++fs)
endif()

# Collect source files from directories
file(GLOB CORE_SRC 
    "*.cpp"
)

file(GLOB UI_SRC 
    "UI/*.cpp"
)

file(GLOB WORKFLOW_SRC 
    "Workflow/*.cpp"
)

file(GLOB UTILS_SRC 
    "Utils/*.cpp"
)

file(GLOB RISK_SRC 
    "Risk/*.cpp"
)

file(GLOB ANALYTICS_SRC 
    "Analytics/*.cpp"
)

file(GLOB JOURNAL_SRC 
    "Journal/*.cpp"
)

file(GLOB BACKTEST_SRC 
    "Backtest/*.cpp"
)

file(GLOB MODELS_SRC 
    "Models/*.cpp"
)

# Add the executable
add_executable(TradingCalculator 
    ${CORE_SRC}
    ${UI_SRC}
    ${WORKFLOW_SRC}
    ${UTILS_SRC}
    ${RISK_SRC}
    ${ANALYTICS_SRC}
    ${JOURNAL_SRC}
    ${BACKTEST_SRC}
    ${MODELS_SRC}
)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/UI)
include_directories(${PROJECT_SOURCE_DIR}/Workflow)
include_directories(${PROJECT_SOURCE_DIR}/Utils)
include_directories(${PROJECT_SOURCE_DIR}/Risk)
include_directories(${PROJECT_SOURCE_DIR}/Analytics)
include_directories(${PROJECT_SOURCE_DIR}/Journal)
include_directories(${PROJECT_SOURCE_DIR}/Backtest)
include_directories(${PROJECT_SOURCE_DIR}/Models)

# Link libraries
target_link_libraries(TradingCalculator PRIVATE
    nlohmann_json::nlohmann_json
)

if(USE_MATPLOTPP)
    target_link_libraries(TradingCalculator PRIVATE MatPlot++::MatPlot++)
endif()

if(USE_CAIRO)
    target_link_libraries(TradingCalculator PRIVATE Cairo::Cairo)
endif()

# Add compiler warnings
if(MSVC)
    target_compile_options(TradingCalculator PRIVATE /W4)
else()
    target_compile_options(TradingCalculator PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Create necessary directories
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/exports/charts)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/logs)

# Install targets
install(TARGETS TradingCalculator DESTINATION bin)
install(DIRECTORY config/ DESTINATION config)
install(DIRECTORY exports/ DESTINATION exports)

# Testing
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Dependencies
include(FetchContent)

# Add spdlog for logging
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.11.0
)
FetchContent_MakeAvailable(spdlog)

# Add MatPlot++ for plotting
FetchContent_Declare(
    matplotplusplus
    GIT_REPOSITORY https://github.com/alandefreitas/matplotplusplus.git
    GIT_TAG v1.1.0
)
set(MATPLOTPP_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(MATPLOTPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(matplotplusplus)

# Add nlohmann_json for JSON parsing
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# Add Catch2 for testing
FetchContent_Declare(
    catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.3.2
)
FetchContent_MakeAvailable(catch2)

# Setup testing
include(CTest)
enable_testing()

# Create test executable
add_executable(unit_tests 
    tests/test_main.cpp
    tests/test_backtester.cpp
    tests/test_batch_backtester.cpp
    Trade.cpp
    Utils.cpp
    Backtest/Backtester.cpp
    Backtest/BatchBacktester.cpp
)

target_link_libraries(unit_tests PRIVATE 
    Catch2::Catch2WithMain
    spdlog::spdlog
    matplot++
    nlohmann_json::nlohmann_json
)

# Register tests
include(${catch2_SOURCE_DIR}/extras/Catch.cmake)
catch_discover_tests(unit_tests) 