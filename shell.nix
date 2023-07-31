{ pkgs ? import <nixpkgs> { } }:
pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    cmake
    ninja
    clang_16
    pkg-config
  ];

  buildInputs = with pkgs; [
    libGL.dev
    microsoft_gsl
    glm
    SDL2.dev
    fmt_9
  ];

  LD_LIBRARY_PATH="/run/opengl-driver/lib:/run/opengl-driver-32/lib";
}
