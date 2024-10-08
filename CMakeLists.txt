cmake_minimum_required(VERSION 3.20)

project(opa)

set(QT_MIN_VERSION "6.0.0")
set(KF_MIN_VERSION "6.3.0")
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ONc)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE Debug)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddTests)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets Sql Quick QuickControls2 QuickWidgets Test)

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    ConfigWidgets
    CoreAddons
    Crash
    DBusAddons
    DocTools
    I18n
    XmlGui
    IconThemes
    ItemModels
    Kirigami
)

find_package(SQLite3)
include_directories(${SQLite3_INCLUDE_DIRS})

kde_enable_exceptions()

add_subdirectory(src)
add_subdirectory(icons)

if (BUILD_TESTING)
    find_package(Qt6 ${QT_MIN_VERSION} NO_MODULE COMPONENTS Test)
    add_subdirectory(autotests)
endif()

# Make it possible to use the po files fetched by the fetch-translations step
ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
