set( maindir ${CMAKE_CURRENT_LIST_DIR})
set( Coredir ${CMAKE_CURRENT_LIST_DIR}/Core )
set( ImageDir ${CMAKE_CURRENT_LIST_DIR}/Image )
set( ViewDir ${CMAKE_CURRENT_LIST_DIR}/View)
set( ThirdParty ${CMAKE_CURRENT_LIST_DIR}/ThirdParty)

##Cores
set( CoreSrcs
    ${Coredir}/nsrDataDef.cpp
    ${Coredir}/nsrIndex.cpp
    ${Coredir}/nsrUtility.cpp
    ${Coredir}/nsrStrUtility.c
    ${Coredir}/nsrFilters.cpp
    ${Coredir}/nsrNumBuffer.c
    ${Coredir}/nsrImageBuffer2.cpp

    ${Coredir}/Matlib/nsrMathLib.cpp
    ${Coredir}/Matlib/nsrMathLibInv.cpp
    ${Coredir}/Matlib/nsrQRDecomp.cpp

    ##${Coredir}/GeoLib/Local2Local.cpp
    ##${Coredir}/GeoLib/Local2Spherical.cpp
    ##${Coredir}/GeoLib/Spherical2Local.cpp

    ${Coredir}/RotLib/quatToRotMat.cpp
    ${Coredir}/RotLib/eulerToRotMat.cpp
    ${Coredir}/RotLib/crossMat.cpp
    ${Coredir}/RotLib/calcEuError.cpp
    
	${ThirdParty}/pugixml/pugixml.cpp
)
	
set( TiffSrc 
	#${ThirdParty}/libtiff/tif_apple.c
	${ThirdParty}/libtiff/tif_aux.c
	${ThirdParty}/libtiff/tif_close.c
	${ThirdParty}/libtiff/tif_codec.c
	${ThirdParty}/libtiff/tif_color.c
	${ThirdParty}/libtiff/tif_compress.c
	${ThirdParty}/libtiff/tif_dir.c
	${ThirdParty}/libtiff/tif_dirinfo.c
	${ThirdParty}/libtiff/tif_dirread.c
	${ThirdParty}/libtiff/tif_dirwrite.c
	${ThirdParty}/libtiff/tif_dumpmode.c
	${ThirdParty}/libtiff/tif_error.c
	${ThirdParty}/libtiff/tif_extension.c
	${ThirdParty}/libtiff/tif_fax3.c
	${ThirdParty}/libtiff/tif_fax3sm.c
	${ThirdParty}/libtiff/tif_flush.c
	${ThirdParty}/libtiff/tif_getimage.c
	${ThirdParty}/libtiff/tif_jbig.c
	${ThirdParty}/libtiff/tif_jpeg.c
	${ThirdParty}/libtiff/tif_luv.c
	${ThirdParty}/libtiff/tif_lzw.c
	${ThirdParty}/libtiff/tif_next.c
	${ThirdParty}/libtiff/tif_ojpeg.c
	${ThirdParty}/libtiff/tif_open.c
	${ThirdParty}/libtiff/tif_packbits.c
	${ThirdParty}/libtiff/tif_pixarlog.c
	${ThirdParty}/libtiff/tif_predict.c
	${ThirdParty}/libtiff/tif_print.c
	${ThirdParty}/libtiff/tif_read.c
	${ThirdParty}/libtiff/tif_stream.cxx
	${ThirdParty}/libtiff/tif_strip.c
	${ThirdParty}/libtiff/tif_swab.c
	${ThirdParty}/libtiff/tif_thunder.c
	${ThirdParty}/libtiff/tif_tile.c
	${ThirdParty}/libtiff/tif_unix.c
	${ThirdParty}/libtiff/tif_version.c
	${ThirdParty}/libtiff/tif_warning.c
	#${ThirdParty}/libtiff/tif_win32.c
	${ThirdParty}/libtiff/tif_write.c
	${ThirdParty}/libtiff/tif_zip.c
)

set( AllViewSrc
    ${ViewDir}/nsrGeoLib.cpp
    ${ViewDir}/nsrOSG.cpp
    ${ViewDir}/nsrOsgCommonView.cpp
    ${ViewDir}/nsrOSGUtility.cpp
    ${ViewDir}/sharing_textures.cpp
    ${ViewDir}/nsrOSGShaders.cpp
    ${ViewDir}/nsrImageDrawable.cpp
    ${ViewDir}/nsrImageDrawableParams.cpp
    ${ViewDir}/nsrMapDrawable2.cpp
    ${ViewDir}/nsrObjectDrawable.cpp
    
    ${ThirdParty}/geoid.c
)

set( CamSimsrc    
    ${maindir}/Sim/Init_Sim.cpp
    ${maindir}/Sim/Sim.cpp
    ${maindir}/Sim/Sensors.cpp
    ${maindir}/Sim/nsrPoseSim.cpp
    ${maindir}/Sim/nsrSimParamReader.cpp
    ${maindir}/Sim/nsrRosInterface.cpp

    ${maindir}/Visualize/nsrVisualize.cpp
	${maindir}/Visualize/nsrMatShow.cpp
	${maindir}/Visualize/plot3d.cpp
	${maindir}/Visualize/nsrLinuxKeyBoard2.cpp
    ${maindir}/Visualize/nsrPlot.cpp
)

set( ViewOFPI2src
    ${maindir}/View/nsrOsgCamSimScene.cpp
    ${maindir}/View/nsrOsgView.cpp
)

set( ImageProcOFPI2src
    ${maindir}/ImageProc/nsrImageProc.cpp
)

set( ImageProcOFPI2src
	${ImageProcOFPI2src}
)

set( MainSrc
    ${maindir}/main.cpp
)

set ( AlgorithmSrcIncludes
    ${maindir}
    ${maindir}/View
    ${maindir}/ImageProc
    ${maindir}/Sim
    ${maindir}/Visualize
)
