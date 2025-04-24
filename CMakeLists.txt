cmake_minimum_required(VERSION 3.10)
project(TradingCalculator VERSION 2.0)

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