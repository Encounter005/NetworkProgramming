file(GLOB_RECURSE srcs CONFIGURE_DEPENDS *.hpp *.cpp *.h)
add_library(src STATIC ${srcs})
target_include_directories(src PUBLIC srcs)
