cmake_minimum_required(VERSION 3.20)

project(opa LANGUAGES CXX C)

set(QT_MIN_VERSION "6.0.0")
set(KF_MIN_VERSION "6.4.0")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddTests)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(
  Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED
  COMPONENTS Core
             Gui
             Widgets
             Sql
             Network
             Quick
             QuickControls2
             QuickWidgets
             Pdf
             PdfWidgets
             Concurrent)

find_package(
  KF6 ${KF_MIN_VERSION} REQUIRED
  COMPONENTS ConfigWidgets
             CoreAddons
             Crash
             DBusAddons
             DocTools
             I18n
             XmlGui
             IconThemes
             ItemModels
             Kirigami
             KIO
             TextWidgets
             WidgetsAddons)

find_package(SQLite3 REQUIRED)

find_package(QtNodes REQUIRED)

find_package(KDDockWidgets-qt6 REQUIRED)

find_package(Qt6Keychain REQUIRED)

find_package(QCoro6 REQUIRED COMPONENTS Core Network DBus)
qcoro_enable_coroutines()

find_package(LibXml2 REQUIRED)

kde_enable_exceptions()

add_subdirectory(src)
add_subdirectory(icons)

if(BUILD_TESTING)
  find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Test)
  add_subdirectory(autotests)
endif()

# Make it possible to use the po files fetched by the fetch-translations step
ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES
                         FATAL_ON_MISSING_REQUIRED_PACKAGES)
