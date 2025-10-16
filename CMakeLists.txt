cmake_minimum_required(VERSION 3.10.0)
project(Project1 VERSION 0.1.0 LANGUAGES C CXX)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version" FORCE)

add_executable(Project1 main.cpp bolo/Player.cpp bolo/Player.h bolo/Bullet.cpp bolo/Bullet.h bolo/Wall.cpp bolo/Wall.h bolo/Explosion.cpp bolo/Explosion.h bolo/Base.cpp bolo/Base.h bolo/Enemy.cpp bolo/Enemy.h bolo/Bolo.cpp bolo/Bolo.h )
add_subdirectory(engine)
add_subdirectory(bolo)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


target_include_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/include/ engine/)
#target_include_directories(${PROJECT_NAME} PUBLIC engine/)
#target_link_directories(${PROJECT_NAME} PUBLIC /opt/homebrew/lib/)
#target_link_libraries(${PROJECT_NAME} PRIVATE SFML::Graphics)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE Engine)