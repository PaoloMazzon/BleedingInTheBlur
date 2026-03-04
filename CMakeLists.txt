# Basic project info
cmake_minimum_required(VERSION 3.20)
project(BleedingInTheBlur)
set(CMAKE_C_STANDARD 23)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(extern/Octarine/OctarineEngine)
include_directories(extern/Octarine/OctarineEngine/include include/)

add_executable(${PROJECT_NAME}
        main.c
        src/Game.c
        src/Constants.c
        src/Character.c
        src/Util.c
        src/Menu.c
        src/Level.c
        src/Popup.c
        src/NPC.c
        src/Player.c
)
target_link_libraries(${PROJECT_NAME} OctarineEngine)