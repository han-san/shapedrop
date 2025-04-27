{
  description = "A game about dropping shapes.";
  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs/nixos-unstable";
    flake-utils = {
      url = "github:numtide/flake-utils";
    };
  };

  outputs = { nixpkgs, flake-utils, ... }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShell =
          pkgs.mkShell {
            # Dependencies that are required during the build.
            nativeBuildInputs = with pkgs; [
              cmake
              ninja
              llvmPackages_21.clang-tools
              clang_21
              pkg-config
            ];

            # Dependencies that are required at runtime.
            buildInputs = with pkgs; [
              libGL.dev
              microsoft-gsl
              glm
              SDL2.dev
              fmt
            ];

            LD_LIBRARY_PATH = "/run/opengl-driver/lib:/run/opengl-driver-32/lib";
          };
      }
    );
}

