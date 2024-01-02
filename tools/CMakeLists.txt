configure_file(
  ofono-setup.in
  ofono-setup
  @ONLY
)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ofono-setup DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS phone-gsettings-migration.py DESTINATION ${CMAKE_INSTALL_BINDIR})
