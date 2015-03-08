project(ktexteditor_keyrecorder)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${QT_INCLUDES})
add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-std=c++11 -Wall -Wextra)

set(ktexteditor_keyrecorder_SRCS keyrecorderplugin.cpp)

kde4_add_plugin(ktexteditor_keyrecorder ${ktexteditor_keyrecorder_SRCS})

target_link_libraries(ktexteditor_keyrecorder ${KDE4_KDEUI_LIBS} ${KDE4_KTEXTEDITOR_LIBS})

install(TARGETS ktexteditor_keyrecorder DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES keyrecorderui.rc DESTINATION ${DATA_INSTALL_DIR}/ktexteditor_keyrecorder)

install(FILES ktexteditor_keyrecorder.desktop DESTINATION ${SERVICES_INSTALL_DIR})

