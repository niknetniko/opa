{
  description = "Experimental Qt/KDE genealogy program";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/x86_64-linux";
    utils = {
      url = "github:numtide/flake-utils";
      inputs.systems.follows = "systems";
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
      treefmt-nix,
      ...
    }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs {
          system = system;
          config = {
            allowUnfree = true;
          };
        };
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
          kdePackages.qtbase
          kdePackages.qtwayland
          kdePackages.qtdeclarative
          kdePackages.qtwebengine
          kdePackages.qcoro
          qtnodes
          kddockwidgets-kde
          sqlite
          kdePackages.qtmultimedia
          kdePackages.qtkeychain
          pipewire
        ];
        native-build-inputs = with pkgs; [
          llvmPackages.openmp
          kdePackages.wrapQtAppsHook
          kdePackages.qtwayland
          kdePackages.extra-cmake-modules
          clang-tools
          cmake
          git
          valgrind
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
          version = "3.0.16";

          src = pkgs.fetchFromGitHub {
            owner = "paceholder";
            repo = "nodeeditor";
            rev = "${version}";
            hash = "sha256-Y9W6CtNB36l2vzWDvlC0BnHJSNe/dN0wW7Pct+/d+1Q=";
          };

          cmakeFlags = [ "-DUSE_QT6=ON" ];

          dontWrapQtApps = true;

          nativeBuildInputs = with pkgs; [ cmake ];
          buildInputs = with pkgs; [ kdePackages.qtbase ];
        };
        kddockwidgets-kde =
          pkgs.kdePackages.callPackage "${pkgs.path}/pkgs/by-name/kd/kddockwidgets/package.nix"
            { };
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
              installPhase = "mkdir -p $out";
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
              pkgs.gemini-cli
              pkgs.nodejs_24
              pkgs.claude-code
            ];
          shellHook = ''
            export KF5ConfigWidgets_DIR=${pkgs.kdePackages.kconfigwidgets}
            export KF5KIconThemes_DIR=${pkgs.kdePackages.kiconthemes}
            export QML_DIR=${pkgs.kdePackages.qtdeclarative}
          '';
        };
        formatter = treefmtEval.config.build.wrapper;
      }
    );
}
