project(nconsole)
cmake_minimum_required(VERSION 2.6)

option(BUILD_TESTS  "Build tests"             ON)
option(QT_SUPPORT   "Enable Qt4 support"      ON)
option(GL_SUPPORT   "Enable OpenGL support"   OFF)

include_directories(./include)

set(nconsole-src   src/nConsole.cpp src/nLogger.cpp src/nRCon.cpp)
set(nconsole-deps  )

# Freetype
#----------
find_package        ( Freetype REQUIRED )
include_directories ( ${FREETYPE_INCLUDE_DIRS} )
list                ( APPEND nconsole-deps ${FREETYPE_LIBRARIES} )


# Qt4 support
#-------------
if( QT_SUPPORT )
  find_package(Qt4 4.6.0 COMPONENTS QtCore QtGui QtXml REQUIRED )
  message(STATUS "Using Qt4")
  include(${QT_USE_FILE})
  include_directories ( ${QT_INCLUDES} )
  add_definitions     ( ${QT_DEFINITIONS} )

  set(qt-moc  examples/rcon_qt.hpp)
  #set(qt-moc  examples/main_qt.cpp)
  #set(qt-res   src/qt/resources/resources.qrc)
  
  if(qt-moc)
    QT4_WRAP_CPP(moc-src ${qt-moc})
  endif()
  
  if(qt-res)
    qt4_add_resources(res-src ${qt-res})
  endif()

  # Target
  #--------
  if( NOT TARGET rcon-qt )
    add_executable        (rcon-qt  examples/rcon_qt.cpp
                                    ${moc-src}
                                    ${res-src} )
    target_link_libraries (rcon-qt  nconsole ${QT_LIBRARIES})
  endif()
endif() # Qt4 support

# Enable C++11 support
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++0x")        ## Optimize
endif()

# Build targets
#---------------
if( NOT TARGET nconsole )
  add_library           (nconsole ${nconsole-src} )
  target_link_libraries (nconsole ${nconsole-deps})
endif()


if( NOT TARGET nconsole-simple )
  add_executable        (nconsole-simple  examples/main.cpp )
  target_link_libraries (nconsole-simple  nconsole)
endif()




# Tests
#-------
if(BUILD_TESTS)
  find_package(CxxTest)
  if(CXXTEST_FOUND)
    aux_source_directory(tests  tests-src)
    if( tests-src )
      message("--------------------------------")
      message("-- Building tests")

      include_directories(${CXXTEST_INCLUDE_DIR})
      set(tests_SOURCES           ${tests-src} )
      set(tests_DEPS              nconsole)

      enable_testing()
      foreach(t_src ${tests_SOURCES})
        get_filename_component(t_name ${t_src} NAME_WE)
        message(STATUS "    -${t_name}: ${t_src}")
        if( NOT TARGET ${t_name} )
          cxxtest_add_test( ${t_name} ${t_name}.cxx 
                            ${CMAKE_CURRENT_SOURCE_DIR}/${t_src})
          target_link_libraries (${t_name} ${tests_DEPS})
        endif()
      endforeach()
    endif()
  endif()
endif()
