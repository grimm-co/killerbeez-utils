cmake_minimum_required (VERSION 2.8.8)
project (utils)

include_directories (${PROJECT_SOURCE_DIR}/../jansson/)
include_directories (${PROJECT_SOURCE_DIR}/)
set(UTILS_SRC
	${PROJECT_SOURCE_DIR}/utils.c
	${PROJECT_SOURCE_DIR}/mutator_factory.c
)
source_group("Library Sources" FILES ${UTILS_SRC})

add_library(utils SHARED ${UTILS_SRC})
target_compile_definitions(utils PUBLIC UTILS_EXPORTS)
if (WIN32)
  target_link_libraries(utils Shlwapi)
endif (WIN32)
target_link_libraries(utils jansson)

add_library(utils_object OBJECT ${UTILS_SRC})
if (NOT WIN32)
  set_target_properties(utils_object PROPERTIES COMPILE_FLAGS "-fPIC")
endif (NOT WIN32)
target_compile_definitions(utils_object PUBLIC UTILS_NO_IMPORT)

add_library(utils_static STATIC ${UTILS_SRC})
target_compile_definitions(utils_static PUBLIC UTILS_NO_IMPORT)
target_link_libraries(utils_static jansson_static)
