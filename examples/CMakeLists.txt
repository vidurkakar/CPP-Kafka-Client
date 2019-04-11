cmake_minimum_required(VERSION 2.8)
project(cpp-kafka-osdk)

link_libraries(cppkafka ${RDKAFKA_LIBRARY} ${Boost_LIBRARIES} pthread rt ssl crypto dl z)

include_directories(/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/build/osdk-core)
link_libraries(djiosdk-core)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../include)
include_directories(SYSTEM ${RDKAFKA_INCLUDE_DIR})
include_directories(/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/osdk-core/api/inc)
include_directories(/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/osdk-core/utility/inc)
include_directories(/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/osdk-core/hal/inc)
include_directories(/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/osdk-core/protocol/inc)
include_directories(/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/osdk-core/platform/linux/inc)
include_directories(/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/sample/linux/common)


link_directories()
#FILE(GLOB SOURCE_FILES *.hpp *.cpp)
#FILE(GLOB SOURCE_FILES "/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/sample/linux/common/dji_linux_environment.cpp")
#FILE(GLOB SOURCE_FILES "/home/nvidia/OSDK_NEW_GitHub_Clone/Onboard-SDK/sample/linux/common/dji_linux_helpers.cpp")


FILE(GLOB SOURCE_FILES *.hpp *.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../OSDK_NEW_GitHub_Clone/Onboard-SDK/sample/linux/common/dji_linux_environment.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../OSDK_NEW_GitHub_Clone/Onboard-SDK/sample/linux/common/dji_linux_helpers.cpp
        )

add_custom_target(examples)
macro(create_example example_name)
    string(REPLACE "_" "-" sanitized_name ${example_name})
    add_executable(${sanitized_name} EXCLUDE_FROM_ALL "${example_name}_example.cpp")
    add_dependencies(examples ${sanitized_name})
endmacro()

create_example(producer)
create_example(buffered_producer)
create_example(consumer)
create_example(consumer_dispatcher)
create_example(metadata)
create_example(consumers_information)
create_example(try_consumer)
