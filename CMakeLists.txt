cmake_minimum_required(VERSION 3.10)

project(LxcTestIssueAppArmorNoNewPrivs)

include(GNUInstallDirs)

find_package(PkgConfig REQUIRED)

pkg_check_modules(LXC REQUIRED lxc>=3.1.0)
pkg_check_modules(apparmor REQUIRED libapparmor)

add_definitions(${LXC_CFLAGS_OTHER})
include_directories(${LXC_INCLUDE_DIRS})
link_directories(${LXC_LIBRARY_DIRS})

add_executable(lxc-test-apparmor
    lxc-apparmor.c)

set(EXAMPLE_CONFIG_INSTALL_PATH "${CMAKE_INSTALL_FULL_DATAROOTDIR}/lxc-playground")

add_definitions(-DEXAMPLE_CONFIG_INSTALL_PATH="${EXAMPLE_CONFIG_INSTALL_PATH}/example.conf")
add_definitions(-DEXAMPLE_TEMPLATE_PATH="${EXAMPLE_CONFIG_INSTALL_PATH}/lxc-custom")
target_link_libraries(lxc-test-apparmor ${LXC_LIBRARIES} ${apparmor_LIBRARIES})

install(TARGETS lxc-test-apparmor DESTINATION bin)
install(FILES example.conf DESTINATION ${EXAMPLE_CONFIG_INSTALL_PATH})
install(PROGRAMS lxc-custom DESTINATION ${EXAMPLE_CONFIG_INSTALL_PATH})
install(FILES lxc-test-profile DESTINATION /etc/apparmor.d)
