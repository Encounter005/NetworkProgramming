# 获取编译器
find_program(
    PROTOC_CXX
    protoc
    DOC "Protobuf Compiler (protoc)"
    REQUIRED
)



# 需要编译的 proto 文件
file (GLOB PROTO_SOURCE_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/*.proto"
)

set(PROTO_PATH    "${CMAKE_CURRENT_SOURCE_DIR}")
set(PROTO_CXX_OUT "${CMAKE_CURRENT_BINARY_DIR}/gen_cxx")

file(MAKE_DIRECTORY ${PROTO_CXX_OUT})

# 使用 protoc 处理 proto 文件
foreach(input_proto ${PROTO_SOURCE_FILES})
    get_filename_component(DIR ${input_proto} DIRECTORY)
    get_filename_component(FILE_NAME ${input_proto} NAME_WE)


    set(OUTPUT_CXX_HEADER   "${PROTO_CXX_OUT}/${FILE_NAME}.pb.h")
    set(OUTPUT_CXX_SOURCE   "${PROTO_CXX_OUT}/${FILE_NAME}.pb.cc")
    list(APPEND OUTPUT_SOURCES_CXX
        ${OUTPUT_CXX_HEADER} ${OUTPUT_CXX_SOURCE})
endforeach()


add_custom_command(
    OUTPUT  ${OUTPUT_SOURCES_CXX}
    COMMAND ${PROTOC_CXX} --cpp_out=${PROTO_CXX_OUT} --proto_path=${PROTO_PATH} ${PROTO_SOURCE_FILES}
    DEPENDS ${PROTO_SOURCE_FILES}
    WORKING_DIRECTORY ${PROTO_PATH}
    COMMENT "Generate Cpp Protobuf Source Files"
)

add_custom_target(
    compile_cxx_protos
    DEPENDS ${OUTPUT_SOURCES_CXX}
)

# 设置生成源文件包含目录变量供上层引用
set(PROTO_GEN_CXX_INCLUDE_DIRS ${PROTO_CXX_OUT} PARENT_SCOPE)

# 将生成的文件打包为库 proto_gen_cxx
# 程序可以链接到该库

add_library(proto_gen_cxx ${OUTPUT_SOURCES_CXX})
target_link_libraries(proto_gen_cxx protobuf)
add_dependencies(proto_gen_cxx compile_cxx_protos)

