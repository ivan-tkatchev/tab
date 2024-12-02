{
  pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/21.11.tar.gz") {}
} :
with pkgs;
let
  pyenv = python3.withPackages (p: with p; [ 
     markdown 
     pygments 
     (callPackage ./TabLexer/default.nix {}) 
  ]);
in
stdenvNoCC.mkDerivation {
  name = "tab-docs";
  version = "9.3";
  src = ./.;
  nativeBuildInputs = [ gnumake pyenv ];
  buildPhase = ''
    rm *.html
    make
  '';
  installPhase = ''
    mkdir -p $out/website
    cp *.html $out
    cp website/* $out/website
    cp {examples,api}.html $out/website
    cp README.html $out/website/docs.html
  '';
}

#mkShell {
#  buildInputs = [ gnumake pyenv glibcLocales ];
#  LOCALE_ARCHIVE_2_27 = "${glibcLocales}/lib/locale/locale-archive";
#  LANG = "en_US.UTF-8";
#}

