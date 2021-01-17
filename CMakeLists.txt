cmake_minimum_required(VERSION "2.8")
Project ("CamSim")

cmake_policy(SET CMP0004 OLD)##for so files

# Options:  Debug, RelWithDebInfo, Release
##set(CMAKE_BUILD_TYPE Release) #disables asserts
set(CMAKE_BUILD_TYPE Debug) #enables asserts

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})#also search specified directory for Find*.cmake

execute_process( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++11 -pthread")
	 
include(${CMAKE_CURRENT_LIST_DIR}/CMakeFlags.cmake)

if( ${ARCHITECTURE} STREQUAL "armv7l" )
    add_definitions(-fPIC -O3 -mfpu=neon -march=armv7-a ${PROJECT_FLAGS})
endif()

if( ${ARCHITECTURE} STREQUAL "x86_64" )
    add_definitions(-fPIC -O3 -mmmx -msse -msse -msse2 -msse3 -mssse3 ${PROJECT_FLAGS})
endif()

#gprof##############################################################
#to run gprof performance profiler for gcc
#sudo apt-get install python3 graphviz
#sudo pip3 install gprof2dot
#run program compiled with -pg flag
#check gmon.out 2 be created
#sudo chown oem ./ -R (in build dir)
#gprof path/to/your/executable | ../gprof2dot.py | dot -Tpng -o output.png
if(true)
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
else()
	add_definitions(-fomit-frame-pointer) ##enable optimization, this is incompatible with -pg
endif()
####################################################################
##valgrind##########################################################
##install valkyrie
####################################################################
##fsanitize-address#################################################
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
##run: $LD_PRELOAD=/usr/lib/x86_64-linux-gnu/libasan.so.4.0.0 && ./CamSim
##use libasan4 with gcc7 and asan5 with gcc8 
###################################################################
#finding unused functions
#cppcheck --enable=unusedFunction .
###################################################################

include(${CMAKE_CURRENT_LIST_DIR}/CMakeRes.cmake)

add_executable( ${PROJECT_NAME}
    ${CoreSrcs}
    ${ImageSrcs}
    ${AllViewSrc}
    ${IOSrc}
    ${CamSimsrc}
    ${ViewOFPI2src}
    ${ImageProcOFPI2src}
    ${MainSrc}
    ${TiffSrc}
)

install (TARGETS ${PROJECT_NAME} DESTINATION bin)

include_directories(
    ${Coredir}
    ${ThirdParty}
	${ThirdParty}/libtiff
    ${LinuxIOdir}	
    ${AlgorithmSrcIncludes}
)

include(${CMAKE_CURRENT_LIST_DIR}/CMakeLib.cmake)
