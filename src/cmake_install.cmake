# Install script for directory: /home/netpipe/Desktop/IrrNew/pal-code/pal/pal

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibpalx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu" TYPE SHARED_LIBRARY FILES "/home/netpipe/Desktop/IrrNew/pal-code/pal/lib/libpal.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libpal.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libpal.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/x86_64-linux-gnu/libpal.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibpalx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibpal-devx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pal/pal" TYPE FILE FILES
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/Config.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/ConfigStatic.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/ConfigVersion.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/pal.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palActuators.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palActivation.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palBase.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palBodies.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palBodyBase.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palCollision.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palDebugDraw.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palException.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palExtraActuators.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palFactory.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palFluid.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palGeometry.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palLinks.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palMath.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palSensors.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palSettings.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palSoftBody.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palSolver.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palStatic.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palStringable.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palTerrain.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palVehicle.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal/palCharacter.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibpal-devx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pal/framework" TYPE FILE FILES
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/common.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/empty.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/errorlog.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/factory.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/factoryconfig.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/managedmemoryobject.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/os.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/osfs.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/statuscode.h"
    "/home/netpipe/Desktop/IrrNew/pal-code/pal/framework/statusobject.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xlibpal-devx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/pal/pal_i" TYPE FILE FILES "/home/netpipe/Desktop/IrrNew/pal-code/pal/pal_i/hull.h")
endif()

