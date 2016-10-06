cmake_minimum_required(VERSION 3.6)
project(smartoffice_srv)

include_directories(/usr/include/mysql++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMYSQLPP_MYSQL_HEADERS_BURIED -g3")
set (CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp smartoffice-srv.h)
add_executable(smartoffice_srv ${SOURCE_FILES})

find_package(Boost REQUIRED COMPONENTS system regex)

if (Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries (smartoffice_srv ${Boost_LIBRARIES})
endif (Boost_FOUND)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(MySQL++ REQUIRED)

if (MYSQLPP_FOUND)
  include_directories(${MYSQLPP_INCLUDE_DIR})
  target_link_libraries (smartoffice_srv ${MYSQLPP_LIBRARIES})
endif (MYSQLPP_FOUND)

target_link_libraries(smartoffice_srv ${LIBS})