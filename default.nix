{
  pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/23.05.tar.gz") {}
} :
with pkgs;
gcc12Stdenv.mkDerivation {
  name = "tab";
  version = "9.3";
  src = ./.;
  nativeBuildInputs = [ python3 ];
  buildPhase = ''
    make -B
    STATIC_LIBS=-L${glibc.static}/lib make -B tab-static
  '';
  installPhase = ''
    mkdir -p $out/bin
    cp tab tab-static $out/bin
  '';
  doCheck = true;
  checkPhase = ''
    make test
  '';
}
