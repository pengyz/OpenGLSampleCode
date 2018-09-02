cmake_minimum_required(VERSION 3.1.1)

include("cmake/HunterGate.cmake")


HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.4.tar.gz"
    SHA1 "8b2255921208517a55c9533c500131387e3c3dd0"
    LOCAL
)

project (OpenGLSample)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

set(HUNTER_LIBS "")

# hunter_add_package(SDL2)
# find_Package(SDL2 CONFIG REQUIRED)
# set(HUNTER_LIBS ${HUNTER_LIBS} SDL2::SDL2)

# hunter_add_package(SDL_image)
# find_package(SDL_image CONFIG REQUIRED)
# set(HUNTER_LIBS ${HUNTER_LIBS} SDL_image::SDL_image)

# hunter_add_package(SDL_mixer)
# find_package(SDL_mixer CONFIG REQUIRED)
# set(HUNTER_LIBS ${HUNTER_LIBS} SDL_mixer::SDL_mixer)

# hunter_add_package(glew)
# find_package(glew CONFIG REQUIRED)
# set(HUNTER_LIBS ${HUNTER_LIBS} glew::glew)

hunter_add_package(glfw)
find_package(glfw3 CONFIG REQUIRED)
set(HUNTER_LIBS ${HUNTER_LIBS} glfw)

hunter_add_package(glm)
find_package(glm CONFIG REQUIRED)
set(HUNTER_LIBS ${HUNTER_LIBS} glm)

Find_Package(OpenGL REQUIRED)

set (CMAKE_CXX_STANDARD 17)
# endif ()
if(WIN32)
    add_definitions(-DNOMINMAX) # http://stackoverflow.com/questions/5004858/stdmin-gives-error
endif()

option (FA_TREAT_WARNINGS_AS_ERRORS "Treat warnings as errors")

if(UNIX)
    set(DEVILUTION_COMPILER_FLAGS "${DEVILUTION_COMPILER_FLAGS} -Wall -pedantic -Wextra -Wno-unknown-pragmas -Wno-missing-braces")
    SET(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")
    if (FA_TREAT_WARNINGS_AS_ERRORS)
        set(DEVILUTION_COMPILER_FLAGS "${DEVILUTION_COMPILER_FLAGS} -Werror")
    endif()
endif()

if(MSVC)
    set(DEVILUTION_COMPILER_FLAGS "${DEVILUTION_COMPILER_FLAGS} /MP /W4 /D_CRT_SECURE_NO_WARNINGS /wd4244 /wd4100 /wd4611 /wd4458 /wd4459 /wd4127 /wd4800 /wd4267 /wd4172")
    # 4244 - e.g. 'argument': conversion from 'const long double' to 'double' -- boost headers
    # 4100 - unreferenced formal parameter -- boost headers
    # 4172 - boost headers
    # 4611 - interaction between '_setjmp' and C++ object destruction is non-portable -- savepng.cpp
    # 4458 - shadow for class member -- librocket headers
    # 4459 - declaration of 'self' hides global declaration
    # 4127 - conditional expression is constant, could be useful to check that both branches compilable.
    # 4267 -  conversion from 'size_t' to 'int', possible loss of data -- too many such warnings in 64-bit build currently
    # 4800 - forcing value to bool, stupid warning
    set(DEVILUTION_COMPILER_FLAGS "${DEVILUTION_COMPILER_FLAGS} /w44062")
    # 4061 - not all enumerator values are handled by the switch statement
    add_definitions (-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS )
    # SCL - because current boost is too old and emits this warning
    # CRT - something in nuklear.h
    if (FA_TREAT_WARNINGS_AS_ERRORS)
        set(DEVILUTION_COMPILER_FLAGS "${DEVILUTION_COMPILER_FLAGS} /WX")
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

include_directories(3rdParty/glad/include Renderer)

add_library(glad STATIC
	3rdParty/glad/include/glad/glad.h
	3rdParty/glad/src/glad.c

)

# Renderer
add_library(Renderer STATIC
    renderer/shader.cpp
    renderer/shader.h
    renderer/stb_image.cpp
    renderer/stb_image.h
)
target_link_libraries(Renderer ${HUNTER_LIBS})


add_executable(LightSample
	main.cpp
)

target_link_libraries(LightSample PUBLIC Renderer glad ${HUNTER_LIBS})
if(WIN32)
    set_target_properties(LightSample PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()