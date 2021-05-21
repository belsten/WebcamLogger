###########################################################################
## $Id: IncludeExtension.cmake 5828 2018-11-28 16:45:06Z abelsten $
## Authors: jezhill@gmail.com

if( MSVC )

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
  set( OPENCV_FFMPEG opencv_videoio_ffmpeg451_64 )
  set( OPENCV_ARCH x64 )
else()
  set( OPENCV_FFMPEG opencv_videoio_ffmpeg451 )
  set( OPENCV_ARCH x32 )
endif()

set( OPENCV_LIBDIR 
  ${PROJECT_SRC_DIR}/extlib/opencv/lib/msvc/${OPENCV_ARCH}
)

list( APPEND BCI2000_SIGSRC_FILES
   ${PROJECT_SRC_DIR}/extlib/opencv/include
   ${BCI2000_EXTENSION_DIR}/WebcamLogger.cpp
   ${BCI2000_EXTENSION_DIR}/WebcamThread.cpp
)

list( APPEND BCI2000_SIGSRC_LIBS 
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_core451$<$<CONFIG:Debug>:d>.lib
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_highgui451$<$<CONFIG:Debug>:d>.lib
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_imgcodecs451$<$<CONFIG:Debug>:d>.lib
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_imgproc451$<$<CONFIG:Debug>:d>.lib
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_video451$<$<CONFIG:Debug>:d>.lib
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_videoio451$<$<CONFIG:Debug>:d>.lib
)

list( APPEND BCI2000_SIGSRC_FILES 
  ${OPENCV_LIBDIR}/${OPENCV_FFMPEG}.dll
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_core451$<$<CONFIG:Debug>:d>.dll
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_highgui451$<$<CONFIG:Debug>:d>.dll
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_imgcodecs451$<$<CONFIG:Debug>:d>.dll
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_imgproc451$<$<CONFIG:Debug>:d>.dll
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_video451$<$<CONFIG:Debug>:d>.dll
  ${OPENCV_LIBDIR}/$<$<CONFIG:Debug>:debug/>opencv_videoio451$<$<CONFIG:Debug>:d>.dll 
)

else( MSVC )

  utils_warn( "WebcamLogger: OpenCV libraries are only present for MSVC on Windows." )

endif( MSVC )



