set(library_SRCS
    accountentry.cpp
    accountentryfactory.cpp
    accountlist.cpp
    audiooutput.cpp
    applicationutils.cpp
    callentry.cpp
    callmanager.cpp
    callnotification.cpp
    channelobserver.cpp
    chatmanager.cpp
    chatentry.cpp
    contactutils.cpp
    contactwatcher.cpp
    greetercontacts.cpp
    ofonoaccountentry.cpp
    participant.cpp
    phoneutils.cpp
    protocol.cpp
    protocolmanager.cpp
    ringtone.cpp
    rolesinterface.cpp
    telepathyhelper.cpp
    tonegenerator.cpp
    ussdmanager.cpp
    )

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${LibPhoneNumber_INCLUDE_DIRS}
    ${URL_DISPATCHER_INCLUDE_DIRS})

add_library(lomiritelephonyservice STATIC ${library_SRCS} ${library_HDRS})
set_target_properties(lomiritelephonyservice PROPERTIES COMPILE_DEFINITIONS AS_BUSNAME=systemBus)
target_link_libraries(lomiritelephonyservice
                      ${TP_QT5_LIBRARIES}
                      ${NOTIFY_LIBRARIES}
                      ${LibPhoneNumber_LIBRARIES}
                      ${URL_DISPATCHER_LIBRARIES}
                      Qt5::Contacts
                      Qt5::Core
                      Qt5::DBus
                      Qt5::Feedback
                      Qt5::Multimedia
                      Qt5::Qml
                      Qt5::Quick
                      Qt5::Gui
                      )

enable_coverage(lomiritelephonyservice)
