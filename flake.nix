{
  inputs = {
    nixkpgs.url = "github:NixOS/nixpkgs/nixos-24.11";
  };
  outputs = { nixpkgs, ... }:
    let
      packages = system:
        let
          pkgs = import nixpkgs { localSystem = system; };
          tab = import ./default.nix { inherit pkgs; };
          docs = import ./docs/default.nix { inherit pkgs; };
        in {
          inherit tab docs;
        };
    in {
      packages.x86_64-linux = packages "x86_64-linux";
      packages.aarch64-linux = packages "aarch64-linux";
    };
}
