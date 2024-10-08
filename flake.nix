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
                clang-tools
                clang
                cmake
                git
                valgrind
                extra-cmake-modules
                kdePackages.kcoreaddons
                kdePackages.kconfigwidgets
                kdePackages.ki18n
                kdePackages.kcrash
                kdePackages.kdbusaddons
                kdePackages.kxmlgui
                kdePackages.appstream-qt
                kdePackages.kirigami
                kdePackages.kiconthemes
                qtcreator
                qt6.qtbase
                qt6.qtwayland
                qt6.qtdeclarative
                clazy
                kdePackages.breeze-icons
                kdePackages.breeze
                kdePackages.kitemmodels
                atlas
                gdb
              ];

              shellHook = ''
                export KF5ConfigWidgets_DIR=${pkgs.kdePackages.kconfigwidgets}
                export KF5KIconThemes_DIR=${pkgs.kdePackages.kiconthemes}
                export QML_DIR=${pkgs.qt6.qtdeclarative}
              '';
          };
        }
    );
}
