foreach(t test_axis test_hist)
    add_executable(${t} ${t}.cxx)
    target_link_libraries(${t} yaml-cpp spdlog::spdlog)
    add_test(NAME ${t}
             COMMAND ${t})
endforeach()