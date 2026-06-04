cmake_minimum_required(VERSION 3.20)
project(TCPMonopoly LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Network
    Svg
    Multimedia
    Test
)

include_directories(
    include
    include/ui
    include/network
)

file(GLOB_RECURSE SOURCES
    src/*.cpp
)

file(GLOB_RECURSE HEADERS
    include/**/*.hpp
)

qt_add_resources(RESOURCES
    resources.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

if(MINGW)
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-mwindows>
    )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
    Qt6::Gui
    Qt6::Svg
    Qt6::Multimedia
)

enable_testing()

add_executable(TCPMonopolyRegressionTests
    tests/regression_tests.cpp

    src/game/game_controller.cpp
    src/game/game_rules.cpp
    src/game/game_session.cpp
    src/game/game_state.cpp
    src/game/models/business_group_utils.cpp

    include/game/game_controller.hpp
    include/game/game_rules.hpp
    include/game/game_session.hpp
    include/game/game_state.hpp
)

target_include_directories(TCPMonopolyRegressionTests PRIVATE
    include
    include/ui
    include/network
)

target_link_libraries(TCPMonopolyRegressionTests PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
)

add_test(
    NAME TCPMonopolyRegressionTests
    COMMAND TCPMonopolyRegressionTests
)