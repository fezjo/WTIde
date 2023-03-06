set(_CMAKE_TOOLCHAIN_PREFIX "llvm-")
message(STATUS "CMake is configured to use the ${_CMAKE_TOOLCHAIN_PREFIX} toolchain")

string(APPEND CMAKE_CXX_FLAGS " -g")
string(APPEND CMAKE_C_FLAGS " -g")
add_link_options(-fuse-ld=lld)

# Ensure Debug Flags
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG")

option(ENABLE_COVERAGE "Enable coverage" OFF)
option(ENABLE_SANS "Enable all sanitizers (except Thread)" OFF)
option(ENABLE_ASAN "Enable Address Sanitizer" OFF)
option(ENABLE_LSAN "Enable Leak Sanitizer" OFF)
option(ENABLE_MSAN "Enable Memory Sanitizer" OFF)
option(ENABLE_UBSAN "Enable Undefined Behavior Sanitizer" OFF)
option(ENABLE_TSAN "Enable Thread Sanitizer" OFF)



# ------------------------------------------------------------------------------
# Coverage
# ------------------------------------------------------------------------------
if(ENABLE_COVERAGE)
    string(APPEND CMAKE_CXX_FLAGS " -O0")
    string(APPEND CMAKE_CXX_FLAGS " -fprofile-arcs -ftest-coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# ------------------------------------------------------------------------------
# Google Sanitizers
# ------------------------------------------------------------------------------

if(ENABLE_SANS)
    set(ENABLE_ASAN ON)
    set(ENABLE_LSAN ON)
    set(ENABLE_UBSAN ON)
endif()

if(ENABLE_ASAN)
    string(APPEND CMAKE_CXX_FLAGS " -O1")
    string(APPEND CMAKE_CXX_FLAGS " -fsanitize=address")
    string(APPEND CMAKE_CXX_FLAGS
        " -fno-omit-frame-pointer"
        " -fno-optimize-sibling-calls"
    )
endif()

if(ENABLE_LSAN)
    string(APPEND CMAKE_CXX_FLAGS " -fsanitize=leak")
endif()

if(ENABLE_MSAN)
    string(APPEND CMAKE_CXX_FLAGS " -O1")
    string(APPEND CMAKE_CXX_FLAGS " -fsanitize=memory")
    string(APPEND CMAKE_CXX_FLAGS
        " -fno-omit-frame-pointer"
        " -fno-optimize-sibling-calls"
    )
endif()

if(ENABLE_UBSAN)
    string(APPEND CMAKE_CXX_FLAGS " -fsanitize=undefined")
    string(APPEND CMAKE_CXX_FLAGS
        " -fsanitize=float-divide-by-zero"
        " -fsanitize=unsigned-integer-overflow"
        " -fsanitize=implicit-conversion"
        " -fsanitize=local-bounds"
    )
endif()

if(ENABLE_TSAN)
    string(APPEND CMAKE_CXX_FLAGS " -fsanitize=thread")
endif()

# ------------------------------------------------------------------------------
# Valgrind
# ------------------------------------------------------------------------------

set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")

message(STATUS "System: ${CMAKE_SYSTEM}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Flags: ${CMAKE_CXX_FLAGS}")
message(STATUS "Debug Flags: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Release Flags: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "RelWithDebInfo Flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "Arch: ${PROCESSOR_ARCH}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
