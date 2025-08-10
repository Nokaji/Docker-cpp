set(DOCKER_SOURCES
        docker.cpp
        container.cpp
        container_manager.cpp
        docker_compose_parser.cpp
        network_manager.cpp
        image_manager.cpp
)

set(DOCKER_HEADERS
        docker.h
        container.h
        container_manager.h
        docker_compose_parser.h
        network_manager.h
        types/container_types.h
        types/network_types.h
        types/image_types.h
        image_manager.h
)

add_library(docker STATIC ${DOCKER_SOURCES} ${DOCKER_HEADERS})

find_package(CURL REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)

target_link_libraries(docker
        PUBLIC utils common_deps
        PRIVATE CURL::libcurl
        PRIVATE fmt::fmt-header-only
        PRIVATE yaml-cpp::yaml-cpp
)

target_include_directories(docker
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${CMAKE_SOURCE_DIR}
)

# Spécifique Windows
if (WIN32)
    target_link_libraries(docker PRIVATE ws2_32)
endif ()