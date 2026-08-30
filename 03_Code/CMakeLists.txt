cmake_minimum_required(VERSION 3.10)
project(ktm_drehzahlmesser_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)

# --- LVGL v8 ---
set(LV_CONF_INCLUDE_SIMPLE ON CACHE BOOL "" FORCE)
set(LV_CONF_PATH ${CMAKE_SOURCE_DIR}/lib/lv_conf.h CACHE STRING "" FORCE)
add_subdirectory(lib/lvgl)

# --- lv_drivers (SDL backend) ---
file(GLOB LV_DRIVERS_SDL_SRC lib/lv_drivers/sdl/*.c)

add_executable(sim
    src/platform/sim/main.c
    src/platform/sim/sim_provider.c
    src/core/ui.c
    ${LV_DRIVERS_SDL_SRC}
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
