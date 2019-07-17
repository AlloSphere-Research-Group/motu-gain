cmake_minimum_required(VERSION 3.0)

# name of app. replace 'test' with app name
set(APP_NAME test)

project(${APP_NAME})


# add allolib as a subdirectory to the project
add_subdirectory(allolib)
add_subdirectory(curl)

# path to main source file
add_executable(${APP_NAME} src/test.cpp)

# link allolib to project
target_link_libraries(${APP_NAME} PRIVATE al libcurl)

# binaries are put into the ./bin directory by default
set (BIN_DIR "${CMAKE_CURRENT_LIST_DIR}/bin")
set_target_properties(${APP_NAME} PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_DEBUG "${BIN_DIR}"
  RUNTIME_OUTPUT_DIRECTORY_RELEASE "${BIN_DIR}"
)
