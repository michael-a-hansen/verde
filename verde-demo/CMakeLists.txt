add_executable(demo demo.cpp)
target_link_libraries(demo yaml-cpp verde)

install(TARGETS demo RUNTIME DESTINATION verde)