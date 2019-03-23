
set(BREP_SOURCE_FILES 
brep.cpp 
)

set(BREP_HEADER_FILES
"${CMAKE_SOURCE_DIR}/include/padt/brep/brep.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/vertex.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/edge.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/face.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/shell.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/loop.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/body.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/part.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/assembly.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/entity.h"
"${CMAKE_SOURCE_DIR}/include/padt/brep/handle.h")


add_library(BRep 
${BREP_HEADER_FILES}
${BREP_SOURCE_FILES}
)

target_include_directories(BRep PUBLIC "${CMAKE_SOURCE_DIR}/include/padt/brep")
target_link_libraries(BRep PRIVATE protobuf::libprotoc protobuf::libprotobuf)