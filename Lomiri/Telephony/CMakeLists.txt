# QML plugin

set(plugin_SRCS
    presencerequest.cpp
    participantsmodel.cpp
    components.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/liblomiritelephonyservice
    )

add_library(lomiritelephonyservice-qml SHARED ${plugin_SRCS} ${plugin_HDRS})

target_link_libraries(lomiritelephonyservice-qml
    ${TP_QT5_LIBRARIES}
    lomiritelephonyservice
    Qt5::Contacts
    Qt5::Core
    Qt5::Qml
    Qt5::Quick
    )

enable_coverage(lomiritelephonyservice-qml)

configure_file(qmldir.in ${CMAKE_CURRENT_BINARY_DIR}/qmldir)
set(PLUGIN_DIR ${QT_INSTALL_QML}/Lomiri/Telephony)
install(TARGETS lomiritelephonyservice-qml DESTINATION ${PLUGIN_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/qmldir DESTINATION ${PLUGIN_DIR})

add_subdirectory(PhoneNumber)
