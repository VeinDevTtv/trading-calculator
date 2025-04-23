cmake_minimum_required(VERSION 3.10)
project(TradingCalculator VERSION 1.0)

# Set C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable
add_executable(TradingCalculator 
    main.cpp
    TradeCalculator.cpp
    Utils.cpp
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