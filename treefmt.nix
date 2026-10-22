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
    "--style=file:./.clang-format"
  ];
  programs.clang-format.package = pkgs.llvmPackages_23.clang-tools;
  # SQL
  programs.sql-formatter.enable = true;
  settings.formatter.sql-formatter.dialect = "sqlite";
}
