add_executable(test_functions test.cpp)
target_link_libraries(
    test_functions
    ${JSONCPP_LIBRARIES}
    ${Boost_LIBRARIES}
    proto_gen
)
