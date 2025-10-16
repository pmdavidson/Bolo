add_library(Engine SHARED GameEngine.cpp GameEngine.h GameObject.cpp GameObject.h DrawContext.cpp DrawContext.h EngineView.h MathUtil.h NotificationManager.cpp NotificationManager.h )

# TODO: Set your required components here... (also see below at target_link_libraries)
set(SFML_COMPONENTS Graphics)

find_package(SFML 3 COMPONENTS ${SFML_COMPONENTS})

if(SFML_FOUND)
    message(STATUS "Found system installation of SFML in ${SFML_DIR}")
else()
    message(STATUS "System installation of SFML not found, looking for install in extern 
                    by appending ${CMAKE_SOURCE_DIR}/extern/SFML-3.0.0 to CMAKE_PREFIX_PATH...")
    list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/extern/SFML-3.0.0")
    find_package(SFML 3 REQUIRED COMPONENTS ${SFML_COMPONENTS})
endif()

target_include_directories(Engine INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Engine PUBLIC /opt/homebrew/include/)
target_compile_features(Engine PRIVATE cxx_std_20)
# TODO: Also update here
target_link_libraries(Engine PUBLIC SFML::Graphics)