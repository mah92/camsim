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

##ROS###################
find_package(catkin COMPONENTS roscpp rosbag std_msgs sensor_msgs geometry_msgs)

if (catkin_FOUND)
    message("-- ROS found!")
    add_definitions(-DROS_FOUND)
    include_directories(${catkin_INCLUDE_DIRS})
    target_link_libraries( ${PROJECT_NAME} 	${catkin_LIBRARIES})
else (catkin_FOUND)
	message("-- ROS not found!")
endif (catkin_FOUND)
 
