cmake_minimum_required(VERSION 3.10)
project(TradingCalculator VERSION 2.0)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# For Windows, add console subsystem to show console window
if(WIN32)
    set_target_properties(TradingCalculator PROPERTIES
        WIN32_EXECUTABLE FALSE
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif() 

# Add compiler warnings
if(MSVC)
    target_compile_options(TradingCalculator PRIVATE /W4)
else()
    target_compile_options(TradingCalculator PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Install targets
install(TARGETS TradingCalculator DESTINATION bin) 