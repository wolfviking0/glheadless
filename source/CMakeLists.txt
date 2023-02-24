
# 
# Configuration for all sub-projects
# 

# Generate version-header
string(TOUPPER ${META_PROJECT_NAME} META_PROJECT_NAME_UPPER)
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME}/${META_PROJECT_NAME}-version.h)


# SET CMAKE VARIABLE
if(NOT WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -fpermissive")
else(NOT WIN32)
    # Ignore warnings about missing pdb
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4286 /ignore:4098 /ignore:4099")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4286 /ignore:4098 /ignore:4099")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4286 /ignore:4098 /ignore:4099")

    string(REPLACE /MDd "/MTd" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    string(REPLACE /MD "/MT" CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")
    string(REPLACE /MDd "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE /MD "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
endif()

set(CMAKE_OBJECT_PATH_MAX 200)
set(CMAKE_USE_RELATIVE_PATHS TRUE)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# 
# Sub-projects
# 

# Libraries
set(IDE_FOLDER "")
add_subdirectory(glheadless)

# Examples
set(IDE_FOLDER "Examples")
add_subdirectory(examples)

# Tests
set(IDE_FOLDER "Tests")
add_subdirectory(tests)


# 
# Deployment
# 

# Deploy generated headers
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/${META_PROJECT_NAME} DESTINATION include COMPONENT dev)
