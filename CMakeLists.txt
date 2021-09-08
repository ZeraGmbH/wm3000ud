cmake_minimum_required(VERSION 3.1)

# set project name and language
project(wm3000ud LANGUAGES CXX)

# Automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
# C++ 11 Flag
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set project Version
set(wm3000ud_VERSION_MAJOR "2")
set(wm3000ud_VERSION_MINOR "15")
set(wm3000ud_VERSION_PATCH "0")

set(PROJECT_VERSION "${wm3000ud_VERSION_MAJOR}.${wm3000ud_VERSION_MINOR}.${wm3000ud_VERSION_PATCH}")

# Add cmake functions like feature_summary()
include(FeatureSummary)
include(GNUInstallDirs)

# Find dependecies
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS Core Xml Network CONFIG REQUIRED)
find_package(zerai2c REQUIRED)
find_package(zeramisc REQUIRED)
find_package(zeramath REQUIRED)
find_package(zeradev REQUIRED)

# sum up project Files 
file(GLOB SOURCES
    ${PROJECT_SOURCE_DIR}/*.cpp 
    )


file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h  
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

# create executable
add_executable(wm3000ud
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )

# link dependencies
target_link_libraries(wm3000ud
    PUBLIC
    Qt5::Core
    PRIVATE
    Qt5::Xml
    Qt5::Network
    ZeraClasses::zerai2c
    ZeraClasses::zeramath
    ZeraClasses::zeramisc
    ZeraClasses::zeradev
    )


# announce headers - target perspective
target_include_directories(wm3000ud
    PUBLIC
    $<BUILD_INTERFACE:${MY_LIBRARY_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/wm3000ud>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )



#install wm3000ud to bin 
install(TARGETS wm3000ud
    EXPORT wm3000udExport
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

