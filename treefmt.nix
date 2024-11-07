{ pkgs, ... }:
{
  projectRootFile = "flake.nix";
  # Nix files
  programs.nixfmt-rfc-style.enable = true;
  # CMake files
  programs.cmake-format.enable = false;
  # C++
  programs.clang-format.enable = true;
  settings.formatter.clang-format.options = [
    "-i"
    "--style=file:./.clang-format"
  ];
}
