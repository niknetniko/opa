{
  description = "Experimental KDE genealogy program";

  inputs = {
    nixpkgs.url = github:NixOS/nixpkgs/nixos-unstable;
    utils.url = github:numtide/flake-utils;
  };

  outputs = { self, nixpkgs, utils }:
    utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
        {
          devShell = pkgs.mkShell {
              buildInputs = with pkgs; [
#                 glibc
                clang-tools
                clang
                cmake
#                 glib
#                 gcc10
                git
                valgrind
                extra-cmake-modules
                libsForQt5.kcoreaddons
                libsForQt5.kconfigwidgets
                libsForQt5.ki18n
                libsForQt5.kcrash
                libsForQt5.kdbusaddons
                libsForQt5.kxmlgui
                libsForQt5.appstream-qt
                libsForQt5.kirigami2
                libsForQt5.kiconthemes
                libsForQt5.qt5.qtquickcontrols2
                qtcreator
                qt5.qtbase
                qt5.qtwayland
                qt5.qtdeclarative

                libsForQt5.breeze-icons
                libsForQt5.breeze-qt5
                libsForQt5.kitemmodels
                atlas
              ];

              shellHook = ''
                export KF5ConfigWidgets_DIR=${pkgs.libsForQt5.kconfigwidgets}
                export KF5KIconThemes_DIR=${pkgs.libsForQt5.kiconthemes}
                export QML_DIR=${pkgs.qt5.qtdeclarative}
              '';
          };
        }
    );
}
