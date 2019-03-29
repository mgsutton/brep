
set(BREP_SOURCE_FILES 
brep.cpp 
)

set(BREP_HEADER_FILES
"${CMAKE_SOURCE_DIR}/include/padt/brep/brep.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/brepBuilder.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/vertex.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/edge.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/face.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/shell.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/loop.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/body.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/part.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/assembly.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/entity.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/entityRange.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/handle.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/utility.h")





add_library(BRep STATIC
${BREP_HEADER_FILES}
${BREP_SOURCE_FILES}
)

protobuf_generate(
	LANGUAGE cpp
	APPEND_PATH
	TARGET BRep
	PROTOS "${CMAKE_SOURCE_DIR}/proto/brep.proto")

target_include_directories(BRep PUBLIC "${CMAKE_SOURCE_DIR}/include/padt/brep")
target_include_directories(BRep PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(BRep PUBLIC "${PROTOBUF_INCLUDE_DIRS}")
target_link_libraries(BRep PRIVATE protobuf::libprotoc protobuf::libprotobuf meta range-v3)
target_compile_options(BRep PRIVATE $<$<CXX_COMPILER_ID:MSVC>: -wd4005 -wd4251 -wd4018 -wd4146 -wd4244 -wd4251 -wd4267 -wd4305 -wd4355 -wd4800 -wd4996>)