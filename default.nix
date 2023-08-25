{
  pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/23.05.tar.gz") {}
} :
with pkgs;
gcc12Stdenv.mkDerivation {
  name = "tab";
  version = "9.2";
  src = ./.;
  nativeBuildInputs = [ python3 ];
  buildPhase = ''
    make
    STATIC_LIBS=-L${glibc.static}/lib make tab-static
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
