cmake_minimum_required(VERSION 3.10)
project(ktm_drehzahlmesser_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

# --- LVGL v9 (v9.1.0 exakt gepinnt, siehe tools/setup_deps.sh) ---
set(LV_CONF_INCLUDE_SIMPLE ON CACHE BOOL "" FORCE)
set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/lib/lv_conf.h CACHE STRING "" FORCE)
add_subdirectory(lib/lvgl)

# v9 kompiliert seinen SDL-Treiber (src/drivers/sdl/) als Teil der lvgl-
# Library selbst mit (ueber LV_USE_SDL in lv_conf.h) - kein separates
# lv_drivers-Repo und kein eigenes GLOB dafuer mehr noetig. Die lvgl-Library
# braucht dafuer aber die SDL2-Header beim eigenen Kompilieren.
target_include_directories(lvgl SYSTEM PRIVATE ${SDL2_INCLUDE_DIRS})

add_executable(sim
    src/platform/sim/main.c
    src/platform/sim/sim_provider.c
    src/core/ui.c
    assets/generated/img_rpm_dial.c
    assets/generated/img_rpm_needle.c
    assets/generated/img_clock_dial.c
    assets/generated/img_hour_needle.c
    assets/generated/img_min_needle.c
)

target_include_directories(sim PRIVATE
    lib
    src
    src/core
    src/platform/sim
    assets/generated
    ${SDL2_INCLUDE_DIRS}
)

target_link_libraries(sim PRIVATE lvgl ${SDL2_LIBRARIES} m pthread)
target_compile_definitions(sim PRIVATE LV_CONF_INCLUDE_SIMPLE)
