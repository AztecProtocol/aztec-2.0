set(CMAKE_CXX_COMPILER $ENV{CXX})
set(CMAKE_C_COMPILER $ENV{CC})
if(APPLE_M1)
    set(DISABLE_ASM ON)
endif()
