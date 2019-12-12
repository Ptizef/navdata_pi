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

SET(VERSION_MAJOR "1")
SET(VERSION_MINOR "0")

MESSAGE (STATUS "*** Building ${PACKAGE_NAME} ***")

MESSAGE (STATUS "Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE("cmake/PluginConfigure.cmake")

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
            src/customgrid.h
            src/customgrid.cpp
            src/datatable.h
            src/datatable.cpp
            src/datadialogbase.h
            src/datadialogbase.cpp
            src/icons.h
            src/icons.cpp
            include/ocpn/ocpn_plugin.h
            include/ocpn/vector2D.h
	)

SET(SRC_wxJSON
            include/wx/json_defs.h
            include/wx/jsonreader.h
            include/wx/jsonval.h
            include/wx/jsonwriter.h
            wxJSON/jsonval.cpp
            wxJSON/jsonreader.cpp
            wxJSON/jsonwriter.cpp
	)

INCLUDE_DIRECTORIES(
            src
            include
)

SET(SRC ${SRC_NAVDATA} ${SRC_wxJSON})
ADD_LIBRARY(${PACKAGE_NAME} SHARED ${SRC})

INCLUDE("cmake/PluginInstall.cmake")
INCLUDE("cmake/PluginLocalization.cmake")
INCLUDE("cmake/PluginPackage.cmake")

