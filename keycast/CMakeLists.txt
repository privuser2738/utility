cmake_minimum_required(VERSION 3.16)
project(KeyCast VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Output to dist folder
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/dist)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/dist)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

# Platform-specific settings
if(WIN32)
    # Windows: Hide console window for GUI app
    set(CMAKE_WIN32_EXECUTABLE ON)

    if(MINGW)
        # Static link libgcc and libstdc++ for MinGW
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
    endif()
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/core/settings.cpp
    src/core/application.cpp
    src/ui/mainwindow.cpp
    src/ui/systemtray.cpp
    src/ui/settingsdialog.cpp
    src/ui/connectionwidget.cpp
    src/network/protocol.cpp
    src/network/discovery.cpp
    src/network/server.cpp
    src/network/client.cpp
    src/network/sslconfig.cpp
    src/input/inputcapture.cpp
    src/input/inputinjector.cpp
    src/shortcuts/shortcutmanager.cpp
    src/shortcuts/actionexecutor.cpp
    src/desktop/screencapture.cpp
    src/desktop/remotedesktopwidget.cpp
    src/desktop/remotedesktopwindow.cpp
)

set(HEADERS
    src/core/settings.h
    src/core/application.h
    src/ui/mainwindow.h
    src/ui/systemtray.h
    src/ui/settingsdialog.h
    src/ui/connectionwidget.h
    src/network/protocol.h
    src/network/discovery.h
    src/network/server.h
    src/network/client.h
    src/network/sslconfig.h
    src/input/inputcapture.h
    src/input/inputinjector.h
    src/shortcuts/shortcutmanager.h
    src/shortcuts/actionexecutor.h
    src/desktop/screencapture.h
    src/desktop/remotedesktopwidget.h
    src/desktop/remotedesktopwindow.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/ui
    ${CMAKE_SOURCE_DIR}/src/network
    ${CMAKE_SOURCE_DIR}/src/input
    ${CMAKE_SOURCE_DIR}/src/shortcuts
    ${CMAKE_SOURCE_DIR}/src/desktop
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ws2_32      # Winsock
        user32      # Input hooks
        advapi32    # Security/crypto
        gdi32       # Screen capture
    )
elseif(UNIX AND NOT APPLE)
    # Linux X11 libraries
    find_package(X11 REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${X11_LIBRARIES}
        ${X11_Xi_LIB}       # XInput2
        ${X11_Xtst_LIB}     # XTest
    )
endif()

# Install target
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_SOURCE_DIR}/dist
)
