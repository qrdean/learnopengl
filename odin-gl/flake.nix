{
  description = "Odin GLFW + OpenGL project";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-26.05";
  };

  outputs = { self, nixpkgs }:
    let
      system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      glfw = pkgs.glfw;
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          glfw
          odin
        ];

        shellHook = ''
          export LIBRARY_PATH="${glfw}/lib:$LIBRARY_PATH"
          export LD_LIBRARY_PATH="${glfw}/lib:$LD_LIBRARY_PATH"
        '';
      };
    };
}
