#------------------------------------------------------------------------------
#  Cmake version 
#------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.12.4 FATAL_ERROR)

#-------------------------------------------------------------------------------
#  Project
#-------------------------------------------------------------------------------
set(PROJ_NAME test-ICP)
project(${PROJ_NAME}
		LANGUAGES 		CXX
)

#-------------------------------------------------------------------------------
#  Set output path
#-------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY 		${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY 		${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY 		${PROJECT_BINARY_DIR}/lib)

#-------------------------------------------------------------------------------
#  Setup my library
#-------------------------------------------------------------------------------
include_directories(include)
aux_source_directory(src 	mySRCS)

#-------------------------------------------------------------------------------
#  Setup PCL library
#-------------------------------------------------------------------------------
find_package(PCL 1.8 REQUIRED)
include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

#-------------------------------------------------------------------------------
#  Setup openCV library (core and surface-matching)
#-------------------------------------------------------------------------------
set(OPENCV_LIB_PATH 	"c:/library/opencv-4.1.0-cuda-9.0")
include_directories(${OPENCV_LIB_PATH}/include)
link_directories(${OPENCV_LIB_PATH}/x64/vc14/lib)
set(OPENCV_MODULE_SS 	opencv_surface_matching410)
set(OPENCV_MODULE_CORE 	opencv_core410)

#-------------------------------------------------------------------------------
#  Link all library
#-------------------------------------------------------------------------------
set(LIBRARIES 	${PROJ_NAME})
add_library(
	${LIBRARIES} STATIC 
	${mySRCS}
)
target_link_libraries(
	${LIBRARIES}
	${PCL_LIBRARIES}
	${OPENCV_MODULE_SS}
	${OPENCV_MODULE_CORE}
)

#-------------------------------------------------------------------------------
#  Add subdirectory
#-------------------------------------------------------------------------------
add_subdirectory(app)
