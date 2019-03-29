# Cmake file for the apps

set(BREP_VIEWER_SOURCE_FILES 
main.cpp 
)

set(BREP_VIEWER_HEADER_FILES)

add_executable(BRepViewer 
${BREP_VIEWER_SOURCE_FILES}
${BREP_VIEWER_HEADER_FILES}
)

target_include_directories(BRepViewer PUBLIC "${CMAKE_SOURCE_DIR}/include/padt/brep")
target_include_directories(BRepViewer PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
target_include_directories(BRepViewer PUBLIC "${PROTOBUF_INCLUDE_DIRS}")
target_link_libraries(BRepViewer PRIVATE BRep glfw protobuf::libprotoc protobuf::libprotobuf)
target_compile_options(BRepViewer PRIVATE $<$<CXX_COMPILER_ID:MSVC>: -wd4005 -wd4251 -wd4018 -wd4146 -wd4244 -wd4251 -wd4267 -wd4305 -wd4355 -wd4800 -wd4996>)