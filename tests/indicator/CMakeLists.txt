include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_BINARY_DIR}/../common
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/liblomiritelephonyservice
    ${CMAKE_SOURCE_DIR}/indicator
    ${MESSAGING_MENU_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${TP_QT5_INCLUDE_DIRS}
    ${HISTORY_INCLUDE_DIRS}
    ${GSETTINGS_QT_INCLUDE_DIRS}
    )

if (NOT ("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "aarch64"))
    generate_telepathy_test(MessagingMenuTest
                            SOURCES MessagingMenuTest.cpp messagingmenumock.cpp ${CMAKE_SOURCE_DIR}/indicator/messagingmenu.cpp
                            LIBRARIES ${TP_QT5_LIBRARIES} lomiritelephonyservice mockcontroller telepathytest ${HISTORY_LIBRARIES}
                            TASKS --task xvfb-run -p -a -p ${CMAKE_BINARY_DIR}/indicator/lomiri-indicator-telephony-service
                                   --task-name lomiri-indicator-telephony-service
                                   --wait-for com.lomiri.TelephonyServiceHandler
                                   --ignore-return)
endif()

qt5_add_dbus_interface(
        qt_SRCS
        "${DATA_DIR}/org.freedesktop.Notifications.xml"
        NotificationsInterface
)
generate_telepathy_test(AuthHandlerTest
              SOURCES AuthHandlerTest.cpp ${CMAKE_SOURCE_DIR}/indicator/authhandler.cpp
                      ${CMAKE_SOURCE_DIR}/indicator/notificationmenu.cpp ${qt_SRCS}
              LIBRARIES ${TP_QT5_LIBRARIES} lomiritelephonyservice mockcontroller telepathytest ${HISTORY_LIBRARIES} ${NOTIFY_LIBRARIES})
generate_test(NotificationMenuTest
              SOURCES NotificationMenuTest.cpp ${CMAKE_SOURCE_DIR}/indicator/notificationmenu.cpp
              LIBRARIES ${NOTIFY_LIBRARIES} Qt5::Core Qt5::DBus Qt5::Test
              USE_DBUS TRUE)
