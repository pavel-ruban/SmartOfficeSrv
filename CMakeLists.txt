cmake_minimum_required(VERSION 3.6)
project(smartoffice_srv)

include_directories(/usr/include/mysql++ src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMYSQLPP_MYSQL_HEADERS_BURIED -g3 -lpthread")
set (CMAKE_CXX_STANDARD 11)
set(SOURCE_FILES main.cpp smartoffice-srv.h src/response_handler.cpp src/response_handler.h src/mysql_handler.cpp src/mysql_handler.h src/session.cpp src/session.h src/server.cpp src/server.h src/client.cpp src/client.h src/logger.cpp src/logger.h src/client_asio.cpp src/client_asio.h src/gateway.cpp src/gateway.h src/config.cpp src/config.h)
add_executable(smartoffice_srv ${SOURCE_FILES})

find_package(Boost REQUIRED COMPONENTS system regex thread)

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