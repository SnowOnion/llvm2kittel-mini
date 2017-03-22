cmake_minimum_required(VERSION 3.6)
project(llvm2kittel_mini)

set(CMAKE_CXX_STANDARD 11)

# GMP staff 1/2 start
set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH}
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules"
        )

find_package(GMP REQUIRED)

#include_directories(${GMP_INCLUDE_DIR})
# GMP staff 1/2 end

include_directories(/usr/lib/llvm-3.4/include)

set(SOURCE_FILES
        llvm2kittel.cpp
        Polynomial.cpp
        gmp_kittel.cpp
        Rule.cpp
        Term.cpp
        Constraint.cpp
        ConstraintEliminator.cpp
        Slicer.cpp)
add_executable(llvm2kittel_mini ${SOURCE_FILES})

# GMP stuff 2/2 start
target_link_libraries(llvm2kittel_mini
        ${LLVM_LIBS} ${LLVM_LDFLAGS} ${GMP_LIBRARIES}
        )
# GMP stuff 2/2 end