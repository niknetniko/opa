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
          llvmPackages.openmp
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
          qt6.qtwayland
          clang-tools
          cmake
          git
          valgrind
          extra-cmake-modules
          clazy
          atlas
        ];
        opa = pkgs.clangStdenv.mkDerivation {
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
          checks = rec {
            ctests = opa.overrideAttrs (finalAttrs: previousAttrs: {
              doCheck = true;
              cmakeBuildType = "Debug";
              QT_QPA_PLATFORM = "offscreen";
            });
          };
          devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
              buildInputs = build-inputs ++ native-build-inputs ++ [
                pkgs.qtcreator
                pkgs.gdb
                pkgs.lldb
                # run-clang-tidy is not included in the normal clang?
                # https://github.com/NixOS/nixpkgs/issues/33386
                # TODO: whut
                pkgs.llvmPackages.clang-unwrapped.python
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
