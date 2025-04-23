cmake_minimum_required(VERSION 3.10)
project(TradingCalculator VERSION 2.0)

# Set C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(TradingCalculator 
    main.cpp
    TradeCalculator.cpp
    Utils.cpp
    Trade.cpp
    SessionManager.cpp
)

# Include directories
include_directories(${PROJECT_SOURCE_DIR})

# For Windows, add console subsystem to show console window
if(WIN32)
    set_target_properties(TradingCalculator PROPERTIES
        WIN32_EXECUTABLE FALSE
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif() 