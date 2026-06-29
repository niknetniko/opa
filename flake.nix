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
          inherit system;
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
          libxml2
        ];
        native-build-inputs = with pkgs; [
          kdePackages.wrapQtAppsHook
          kdePackages.extra-cmake-modules
          cmake
        ];
        dev-tools = with pkgs; [
          clang-tools
          clazy
          valgrind
          gdb
          lldb
          qtcreator
          gersemi
          nodejs_24
          claude-code
          bashInteractive
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
            rev = version;
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
        run-tidy = pkgs.writeShellApplication {
          name = "run-tidy";
          runtimeInputs = [ pkgs.clang-tools pkgs.cmake ];
          # TODO: enable on test code later
          text = ''
            cmake -S . -B build-tidy -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
            cmake --build build-tidy --target opa_autogen opa-lib_autogen
            run-clang-tidy -p build-tidy -quiet "$PWD/src/.*"
          '';
        };
        # Wrap clazy-standalone
        clazy-standalone = pkgs.clangStdenv.mkDerivation {
          name = "clazy-standalone-wrapped";
          dontUnpack = true;
          dontWrapQtApps = true;
          buildInputs = build-inputs;
          nativeBuildInputs = [ pkgs.makeWrapper ];
          installPhase = ''
            cc=${pkgs.clangStdenv.cc}
            flags="$NIX_CFLAGS_COMPILE $(cat "$cc/nix-support/libcxx-cxxflags") $(cat "$cc/nix-support/libc-cflags")"
            extra=""
            for flag in $flags; do
              extra="$extra -extra-arg=$flag"
            done
            makeWrapper ${pkgs.clazy}/bin/clazy-standalone $out/bin/clazy-standalone \
              --add-flags "$extra"
          '';
        };
        run-clazy = pkgs.writeShellApplication {
          name = "run-clazy";
          runtimeInputs = [
            pkgs.clang-tools
            clazy-standalone
            pkgs.cmake
          ];
          # TODO: enable on test code later
          text = ''
            cmake -S . -B build-tidy -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
            cmake --build build-tidy --target opa_autogen opa-lib_autogen
            run-clang-tidy -clang-tidy-binary=clazy-standalone -p build-tidy -checks=level1,no-non-pod-global-static "$PWD/src/.*"
          '';
        };
      in
      {
        packages = {
          default = opa;
          qtnodes = qtnodes;
        };
        checks = {
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
              nativeBuildInputs = previousAttrs.nativeBuildInputs ++ [ pkgs.clang-tools ];
              cmakeFlags = [
                # Run using the normal CMake build so we have generated headers etc. in the correct
                # places automatically.
                "-DCMAKE_CXX_CLANG_TIDY=clang-tidy"
              ];
              dontInstall = true;
            }
          );
          clang-clazy = opa.overrideAttrs (
            finalAttrs: previousAttrs: {
              name = "opa-clang-clazy";
              nativeBuildInputs = previousAttrs.nativeBuildInputs ++ [ pkgs.clazy ];
              cmakeFlags = [
                # Run using the normal CMake build so we have generated headers etc. in the correct
                # places automatically.
                "-DCMAKE_CXX_COMPILER=clazy"
              ];
              CLAZY_CHECKS = "level1,no-non-pod-global-static";
              installPhase = "mkdir -p $out";
            }
          );
          formatting = treefmtEval.config.build.check self;
        };
        devShells.default = pkgs.mkShell.override { stdenv = pkgs.clangStdenv; } {
          inputsFrom = [ opa ];
          packages = dev-tools ++ [ treefmtEval.config.build.wrapper run-tidy run-clazy ];
          shellHook = ''
            export QML_DIR=${pkgs.kdePackages.qtdeclarative}
          '';
        };
        formatter = treefmtEval.config.build.wrapper;
      }
    );
}
