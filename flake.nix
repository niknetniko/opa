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
        build-inputs = with pkgs; [
          kdePackages.kcoreaddons
          kdePackages.kconfigwidgets
          kdePackages.ki18n
          kdePackages.kcrash
          kdePackages.kdbusaddons
          kdePackages.kxmlgui
          kdePackages.appstream-qt
          kdePackages.kirigami
          kdePackages.kiconthemes
          kdePackages.breeze-icons
          kdePackages.breeze
          kdePackages.kitemmodels
          qt6.qtbase
          qt6.qtwayland
          qt6.qtdeclarative

        ];
        native-build-inputs = with pkgs; [
          qt6.wrapQtAppsHook
          clang-tools
          clang
          cmake
          git
          valgrind
          extra-cmake-modules
          qtcreator
          clazy
          atlas
          gdb
        ];
        opa = pkgs.stdenv.mkDerivation {
          pname = "opa";
          version = "0.1";
          src = ./.;

          buildInputs = build-inputs;
          nativeBuildInputs = native-build-inputs;
        };
      in
        {
          packages = rec {
            default = opa;
          };
          # TODO: this crashes before tests are even run, so it probably doesn't build correctly.
          checks = rec {
            ctests = opa.overrideAttrs (finalAttrs: previousAttrs: {
              doCheck = true;
              cmakeBuildType = "Debug";
              checkFlags = "--debug";
            });
          };
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
