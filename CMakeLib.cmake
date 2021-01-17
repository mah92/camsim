##for multi touch 
##sudo apt-get install libevdev-dev
##sudo apt-get install libmtdev-dev
##sudo apt-get install xdotool
##sudo apt-get install liblzma-dev
##sudo apt-get install libeigen3-dev
##sudo apt-get install libplplot-dev

#sudo apt-get install libosgearth-dev openscenegraph-plugin-osgearth
#sudo apt-get install libsm-dev
#sudo apt-get install qt5-default


#######################################3
#symbol lookup error: /usr/lib/x86_64-linux-gnu/libQt5XcbQpa.so.5: undefined symbol: FT_Get_Font_Format
#remove /usr/local/lib/libfreetype.so, libfreetype.so.6

########################################3


## install gcc & g++ 4.8 for eigen
##sudo apt-get install python-software-properties
##sudo add-apt-repository ppa:ubuntu-toolchain-r/test
##sudo apt-get update
##sudo apt-get install gcc-4.8
##sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-4.8 50
##sudo apt-get install g++-4.8
##sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-4.8 50
##gcc --version # check gcc and g++ version
##gcc-7 test is OK but gcc-8 is buggy

##both the current program compiler and osg compiler should belong to before or after gcc-5.4, otherwise linking error occurs

##add -llzma to end of target link libraries
##if you got error with mtdev, check if you have run program with sudo

if( ${ARCHITECTURE} STREQUAL "armv7l" )
	target_link_libraries( ${PROJECT_NAME} "/usr/lib/arm-linux-gnueabihf/libX11.so.6" ) #${X11_LIBRARIES})
	target_link_libraries( ${PROJECT_NAME}  -lm)
endif()

if( ${ARCHITECTURE} STREQUAL "x86_64" )
	find_package(X11 REQUIRED )
	target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES} -lm -llzma)
endif()

##PLPLOT#############################
#target_link_libraries( ${PROJECT_NAME} -lplplotcxxd)
target_link_libraries( ${PROJECT_NAME} -lplplotcxx)

include_directories(
	"/usr/include/plplot"
)

##OSG########################
find_package(OpenSceneGraphi 2.0.0 REQUIRED
    osgDB
    osg
    osgUtil
    OpenThreads
)

if(OSG_FOUND)
    target_link_libraries (${PROJECT_NAME}
        ##OSG
        ${OSG_LIBRARY}
        ${OSGGA_LIBRARY}
        ${OSGUTIL_LIBRARY}
        ${OSGDB_LIBRARY}
        ${OSGTEXT_LIBRARY}
        ${OSGWIDGET_LIBRARY}
        ${OSGTERRAIN_LIBRARY}
        ${OSGFX_LIBRARY}
        ${OSGVIEWER_LIBRARY}
        ${OSGVOLUME_LIBRARY}
        ${OSGMANIPULATOR_LIBRARY}
        ${OSGANIMATION_LIBRARY}
        ${OSGPARTICLE_LIBRARY}
        ${OSGSHADOW_LIBRARY}
        ${OSGPRESENTATION_LIBRARY}
        ${OSGSIM_LIBRARY}
        ##OPENTHREADS
        ${OPENTHREADS_LIBRARY}
    )
    message("osg found!")
else(OSG_FOUND)
    message("osg not found!")
endif(OSG_FOUND)

##OpenCV################
find_package( OpenCV REQUIRED )

include_directories(
    "${OpenCV_INCLUDE_DIRS}"
)
target_link_libraries( ${PROJECT_NAME} ${OpenCV_LIBS} )

##PTHREADS##############
find_package ( Threads REQUIRED )
if (THREADS_FOUND)
    target_link_libraries ( ${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )
else (THREADS_FOUND)
    message("Can't find pthreads!")
endif (THREADS_FOUND)

