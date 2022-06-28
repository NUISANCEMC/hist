add_executable(Axis_Tests Axis_Tests.cxx)
target_link_libraries(Axis_Tests Eigen3::Eigen)

add_executable(CountingTensor_Tests CountingTensor_Tests.cxx)
target_link_libraries(CountingTensor_Tests Eigen3::Eigen)

add_executable(TensorVarier_Tests TensorVarier_Tests.cxx)
target_link_libraries(TensorVarier_Tests Eigen3::Eigen)

install(TARGETS Axis_Tests 
	CountingTensor_Tests 
	TensorVarier_Tests 
	DESTINATION bin)