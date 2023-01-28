{
  pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/22.05.tar.gz") {}
} :
with pkgs;
let 
  stdenv = gcc11Stdenv;
in
mkShell.override { inherit stdenv; } {
  buildInputs = [ stdenv.glibc.static python3 ];
}

