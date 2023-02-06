{
  pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/22.11.tar.gz") {}
} :
with pkgs;
mkShell.override { stdenv = gcc12Stdenv; } {
  buildInputs = [ python3 ];
}

