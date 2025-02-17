
set(qt_SRCS
    approver.cpp
    approverdbus.cpp
    )

set(approver_SRCS main.cpp ${qt_SRCS})
qt5_add_dbus_adaptor(approver_SRCS Approver.xml approver/approverdbus.h ApproverDBus)

include_directories(
    ${TP_QT5_INCLUDE_DIRS}
    ${NOTIFY_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/liblomiritelephonyservice
    ${CMAKE_CURRENT_BINARY_DIR}
    )

link_directories(${MESSAGING_MENU_LIBRARY_DIRS})

add_executable(lomiri-telephony-service-approver ${approver_SRCS} ${approver_HDRS})

target_link_libraries(lomiri-telephony-service-approver
    ${TP_QT5_LIBRARIES}
    ${NOTIFY_LIBRARIES}
    lomiritelephonyservice
    Qt5::Contacts
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    Qt5::Multimedia
    Qt5::Qml
    Qt5::Feedback
    )

enable_coverage(lomiri-telephony-service-approver)

configure_file(org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service.in org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service)
install(TARGETS lomiri-telephony-service-approver RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.Telepathy.Client.TelephonyServiceApprover.service DESTINATION share/dbus-1/services)
install(FILES LomiriTelephonyServiceApprover.client DESTINATION share/telepathy/clients)
install(FILES 50-com.lomiri.TelephonyServiceApprover.pkla DESTINATION "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/polkit-1/localauthority/10-vendor.d")
install(FILES com.lomiri.TelephonyServiceApprover.policy DESTINATION share/polkit-1/actions)
install(FILES com.lomiri.TelephonyServiceApprover.xml DESTINATION share/dbus-1/interfaces)

# Create accountsservice symlink for above dbus interface
install(CODE "
execute_process(COMMAND mkdir -p \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/accountsservice/interfaces\")
execute_process(COMMAND ln -sf ../../dbus-1/interfaces/com.lomiri.TelephonyServiceApprover.xml \"\$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/share/accountsservice/interfaces\")
    ")
