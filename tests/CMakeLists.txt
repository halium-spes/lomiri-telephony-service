include (GenerateTest)

add_subdirectory(common)
add_subdirectory(handler)
add_subdirectory(libtelephonyservice)
add_subdirectory(Lomiri.Telephony)

if (WANT_UI_SERVICES)
    add_subdirectory(approver)
    add_subdirectory(indicator)
endif()
