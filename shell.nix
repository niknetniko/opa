with (import <nixpkgs> {});

let
  qt = qt5.full;
in

mkShell {
  buildInputs = [
    clang-tools
    clang
    cmake
    glib
    gcc10
    extra-cmake-modules
    libsForQt5.kcoreaddons
    libsForQt5.kconfigwidgets
    libsForQt5.ki18n
    libsForQt5.kcrash
    libsForQt5.kdbusaddons
    libsForQt5.kxmlgui
    libsForQt5.appstream-qt
    libsForQt5.breeze-icons
    libsForQt5.breeze-qt5
    qtcreator
    qt
  ];
      #export QML2_IMPORT_PATH=${kirigami}/lib/${builtins.replaceStrings ["full-"] [""] qt.name}/qml
  shellHook = ''
    export KF5ConfigWidgets_DIR=${libsForQt5.kconfigwidgets}
  '';
}
