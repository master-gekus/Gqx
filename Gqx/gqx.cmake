# ==================

if(__gqx_included)
  return()
endif()
set(__gqx_included YES)

get_filename_component(GQX_PATH ${CMAKE_CURRENT_LIST_FILE} PATH )
include_directories(${GQX_PATH})

function(gqx_get_modules outfiles)
  set(gqx_files ${${outfiles}})

  set(libs "${ARGV}")
  list(REMOVE_AT libs 0)
  foreach( lib ${libs} )
    if( ${lib} STREQUAL "GJson" )
      __gqx_use_json()
    elseif( ${lib} STREQUAL "GSelfOwnedThread" )
      __gqx_use_self_owned_thread()
    elseif( ${lib} STREQUAL "GLogger" )
      __gqx_use_glogger()
    elseif( ${lib} STREQUAL "GUiHelpers" )
      __gqx_use_ui_helpers()
    elseif( ${lib} STREQUAL "GConsoleWidget" )
      __gqx_use_console_widget()
    else()
      message(FATAL_ERROR "Unknowns Gqx module: ${lib}")
      return()
    endif()
  endforeach()

  list(REMOVE_DUPLICATES gqx_files)

  set(${outfiles} ${gqx_files} PARENT_SCOPE)
endfunction()

macro(__gqx_use_json)
  list(APPEND gqx_files
    ${GQX_PATH}/GJson.cpp
    ${GQX_PATH}/GJson.h
  )
endmacro()

macro(__gqx_use_self_owned_thread)
  list(APPEND gqx_files
    ${GQX_PATH}/GSelfOwnedThread.cpp
    ${GQX_PATH}/GSelfOwnedThread.h
  )
endmacro()

macro(__gqx_use_glogger)
  __gqx_use_self_owned_thread()
  list(APPEND gqx_files
    ${GQX_PATH}/GLogger.cpp
    ${GQX_PATH}/GLoggerEvent.cpp
    ${GQX_PATH}/GLogger.h
    ${GQX_PATH}/GLoggerEvent.h
    ${GQX_PATH}/private/GLogger_p.h
    ${GQX_PATH}/private/GLoggerEvent_p.h
  )
endmacro()

macro(__gqx_use_ui_helpers)
  list(APPEND gqx_files
    ${GQX_PATH}/GUiHelpers.cpp
    ${GQX_PATH}/GUiHelpers.h
  )
endmacro()

macro(__gqx_use_console_widget)
  list(APPEND gqx_files
    ${GQX_PATH}/GConsoleWidget.cpp
    ${GQX_PATH}/GConsoleWidget.h
  )
endmacro()
