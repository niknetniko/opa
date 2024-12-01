{
  description = "Experimental Qt/KDE genealogy program";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/x86_64-linux";
    utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
    };
    nix-github-actions = {
      url = "github:nix-community/nix-github-actions";
      inputs.nixpkgs.follows = "nixpkgs";
    };
    treefmt-nix = {
      url = "github:numtide/treefmt-nix";
      inputs.nixpkgs.follows = "nixpkgs";
    };
  };

  outputs =
    {
      self,
      nixpkgs,
      utils,
      nix-github-actions,
      treefmt-nix,
      ...
    }:
    utils.lib.eachDefaultSystem (
      system:
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
          kdePackages.kio
          qt6.qtbase
          qt6.qtwayland
          qt6.qtdeclarative
          qtnodes
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
        qtnodes = pkgs.clangStdenv.mkDerivation rec {
          pname = "qtnodes";
          version = "3.0.11";

          src = pkgs.fetchFromGitHub {
            owner = "paceholder";
            repo = "nodeeditor";
            rev = "${version}";
            hash = "sha256-Duy9Z+9d12sqwkHQ2eyMI4WrcXCte4M90xNxyrGcOCw=";
          };

          cmakeFlags = [ "-DUSE_QT6=ON" ];

          dontWrapQtApps = true;

          nativeBuildInputs = with pkgs; [ cmake ];
          buildInputs = with pkgs; [ qt6.qtbase ];
        };
        treefmtEval = treefmt-nix.lib.evalModule pkgs ./treefmt.nix;
      in
      {
        packages = {
          default = opa;
          qtnodes = qtnodes;
        };
        checks = rec {
          ctests = opa.overrideAttrs (
            finalAttrs: previousAttrs: {
              name = "opa-ctests";
              doCheck = true;
              cmakeBuildType = "Debug";
              QT_QPA_PLATFORM = "offscreen";
              dontInstall = true;
            }
          );
          clang-tidy = opa.overrideAttrs (
            finalAttrs: previousAttrs: {
              name = "opa-clang-tidy";
              cmakeFlags = [
                "-DCMAKE_CXX_CLANG_TIDY='clang-tidy'"
              ];
              dontInstall = true;
            }
          );
          formatting = treefmtEval.config.build.check self;
        };
        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          buildInputs =
            build-inputs
            ++ native-build-inputs
            ++ [
              pkgs.qtcreator
              pkgs.gdb
              pkgs.lldb
              # run-clang-tidy is not included in the normal clang?
              # https://github.com/NixOS/nixpkgs/issues/33386
              # TODO: whut
              pkgs.llvmPackages.clang-unwrapped.python
              pkgs.gersemi
              treefmtEval.config.build.wrapper
              pkgs.bashInteractive
            ];
          shellHook = ''
            export KF5ConfigWidgets_DIR=${pkgs.kdePackages.kconfigwidgets}
            export KF5KIconThemes_DIR=${pkgs.kdePackages.kiconthemes}
            export QML_DIR=${pkgs.qt6.qtdeclarative}
          '';
        };
        formatter = treefmtEval.config.build.wrapper;
      }
    )
    // utils.lib.eachDefaultSystemPassThrough (system: {
      githubActions = nix-github-actions.lib.mkGithubMatrix { inherit (self) checks; };
    });
}
