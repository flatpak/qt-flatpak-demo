project(flatpak-demo)

cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
set(QT_MIN_VERSION "5.15.2")

################# set KDE specific information #################

find_package(ECM 5.86.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core DBus Quick Widgets QuickControls2)

find_package(KF5 REQUIRED Notifications)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMInstallIcons)
include(ECMPackageConfigHelpers)
include(ECMOptionalAddSubdirectory)

include_directories(${Qt5Core_PRIVATE_INCLUDE_DIRS})

add_subdirectory(icons)

qt5_add_resources(flatpak_demo_SRCS assets.qrc)
qt5_add_resources(flatpak_demo_SRCS resources.qrc)

add_executable(flatpak-demo ${flatpak_demo_SRCS}
    flatpakdemo.cpp
    main.cpp
)

set_target_properties(flatpak-demo PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_FULL_LIBDIR}/flatpak-demo)

target_link_libraries(flatpak-demo LINK_PUBLIC Qt5::QuickControls2 Qt5::Widgets Qt5::Quick Qt5::DBus Qt5::Core KF5::Notifications)

install(TARGETS flatpak-demo ${INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.flatpak.qtdemo.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.flatpak.qtdemo.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES flatpak-print-cheatsheet.pdf DESTINATION ${DATA_INSTALL_DIR}/org.flatpak.qtdemo/)
