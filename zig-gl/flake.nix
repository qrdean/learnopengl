{
  inputs.zig_overlay.url = "github:mitchellh/zig-overlay";
  inputs.zls_overlay.url = "github:zigtools/zls?rev=494486203c3a48927f2383aa3d5ce5fca112186d";

  outputs =
    {
      self,
      zig_overlay,
      zls_overlay,
      nixpkgs,
    }:
    let
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
      zig = zig_overlay.packages.${system}.default;
      zls = zls_overlay.packages.${system}.zls.overrideAttrs (old: {
        nativeBuildInputs = [ zig ];
      });
    in
    {
      devShells.${system}.default = pkgs.mkShell {
        packages = with pkgs; [
          zig
          zls
        ];
        buildInputs = with pkgs; [
          glfw
          libGL
          wayland
          libxkbcommon
          libglvnd
        ];

        shellHook = ''
          export LD_LIBRARY_PATH=${pkgs.wayland}/lib:${pkgs.libxkbcommon}/lib:${pkgs.libglvnd}/lib:$LD_LIBRARY_PATH
        '';
      };
      # LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath (with pkgs; [ wayland libxkbcommon libglvnd ]);
    };
}
