{
  pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/21.11.tar.gz") {}
} :
with pkgs;
mkShell.override { stdenv = gcc11Stdenv; } {
  buildInputs = [ python3 glibc.static ];
}

