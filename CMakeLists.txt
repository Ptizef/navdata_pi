##---------------------------------------------------------------------------
## Author:      Dave Register
##***************************************************************************
## *   Copyright (C) 2010 by David S. Register                               *
## *   This program is free software; you can redistribute it and/or modify  *
## *   it under the terms of the GNU General Public License as published by  *
## *   the Free Software Foundation; either version 2 of the License, or     *
## *   (at your option) any later version.                                   *
## *                                                                         *
## *   This program is distributed in the hope that it will be useful,       *
## *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
## *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
## *   GNU General Public License for more details.                          *
## *                                                                         *
## *   You should have received a copy of the GNU General Public License     *
## *   along with this program; if not, write to the                         *
## *   Free Software Foundation, Inc.,                                       *
## *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.             *
## ***************************************************************************
# define minimum cmake version
CMAKE_MINIMUM_REQUIRED(VERSION 3.1.1)

PROJECT(navdata_pi)

SET(PACKAGE_NAME navdata_pi)
SET(VERBOSE_NAME navdata)
SET(TITLE_NAME navdata)
SET(CPACK_PACKAGE_CONTACT "Jean Pierre Joubert")

SET(VERSION_MAJOR "0")
SET(VERSION_MINOR "2")

MESSAGE (STATUS "*** Building ${PACKAGE_NAME} ***")

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#INCLUDE("cmake/PluginConfigure.cmake")

IF (PREFIX)
	SET(CMAKE_INSTALL_PREFIX ${PREFIX})
ENDIF (PREFIX)

IF(NOT WIN32)
 ADD_COMPILE_OPTIONS("-Wall" "-g" "-fexceptions" "-fvisibility=hidden")

 IF(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-Bsymbolic")
 ELSE(NOT APPLE)
  SET(CMAKE_SHARED_LINKER_FLAGS "-Wl")
 ENDIF(NOT APPLE)

ENDIF(NOT WIN32)

# Add some definitions to satisfy MS
IF(WIN32)
    ADD_DEFINITIONS(-D__MSVC__)
    ADD_DEFINITIONS(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE)
ENDIF(WIN32)


SET(wxWidgets_USE_LIBS base core net xml html adv)
SET(BUILD_SHARED_LIBS TRUE)
FIND_PACKAGE(wxWidgets REQUIRED)

INCLUDE(${wxWidgets_USE_FILE})

FIND_PACKAGE(Gettext REQUIRED)

SET(SRC_NAVDATA
            src/navdata_pi.h
            src/navdata_pi.cpp
            src/datatable.h
            src/datatable.cpp
            src/datadialogbase.h
            src/datadialogbase.cpp
            src/routecanv.h
            src/routecanv.cpp
            src/icons.h
            src/icons.cpp
            ocpn/styles.h
            ocpn/vector2D.h
            ocpn/vector2D.cpp
            ocpn/ocpn_plugin.h
	)

SET(SRC_wxJSON
            wxJSON/json_defs.h
            wxJSON/jsonreader.h
            wxJSON/jsonval.h
            wxJSON/jsonwriter.h
            wxJSON/jsonval.cpp
            wxJSON/jsonreader.cpp
            wxJSON/jsonwriter.cpp
	)

INCLUDE_DIRECTORIES(
            src
            ocpn
            wxJSON
)

IF(UNIX)
 INCLUDE("cmake/FindTinyXML.cmake")
 FIND_PACKAGE(TinyXML QUIET)
ENDIF(UNIX)

IF(TINYXML_FOUND)
 message (STATUS "Building with system tinyxml")
 INCLUDE_DIRECTORIES(${TINYXML_INCLUDE_DIR})

 SET(SRC ${SRC_NAVDATA} ${SRC_wxJSON} ${SRC_ocpn})
 ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC})

 TARGET_LINK_LIBRARIES(${PACKAGE_NAME} ${TINYXML_LIBRARIES})

ELSE(TINYXML_FOUND)
 message (STATUS "Building with embedded tinyxml")
 INCLUDE_DIRECTORIES(src/tinyxml/)
 SET(SRC_LTINYXML
            src/TinyXML/tinyxml.cpp
            src/TinyXML/tinyxmlparser.cpp
            src/TinyXML/tinyxmlerror.cpp
 )
SET(SRC ${SRC_NAVDATA} ${SRC_wxJSON} ${SRC_ocpn} ${SRC_LTINYXML})
ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC})

ENDIF(TINYXML_FOUND)

ADD_DEFINITIONS(-DTIXML_USE_STL)

INCLUDE("cmake/PluginConfigure.cmake")
INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")

