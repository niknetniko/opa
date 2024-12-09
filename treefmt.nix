{ pkgs, ... }:
{
  projectRootFile = "flake.nix";
  # Nix files
  programs.nixfmt.enable = true;
  # CMake files
  programs.cmake-format.enable = true;
  settings.formatter.cmake-format.includes = [
    "**/CMakeLists.txt"
  ];
  # C++
  programs.clang-format.enable = true;
  settings.formatter.clang-format.options = [
    "-i"
    "--style=file:./.clang-format"
  ];
}
