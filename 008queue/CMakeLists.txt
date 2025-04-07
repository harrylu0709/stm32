cmake_minimum_required(VERSION 3.20)

project("008queue" C CXX ASM)

include(cmake/st-project.cmake)
include_directories(./FreeRTOS/include)
include_directories(./FreeRTOS/portable/GCC/ARM_CM4F)
file(GLOB all_SRCS
        "./FreeRTOS/*.c"
        "./FreeRTOS/*.h"
        )
add_executable(${PROJECT_NAME})
add_st_target_properties(${PROJECT_NAME})