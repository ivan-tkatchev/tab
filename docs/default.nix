{
  pkgs ? import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/21.11.tar.gz") {}
} :
with pkgs;
let
  python = python3;
  tablexer = callPackage ./TabLexer { python3Packages = python.packages; };
  pyenv = python.withPackages (p: with p; [ 
     markdown 
     pygments 
     (callPackage ./TabLexer { python3Packages = p; }) 
  ]);
in
mkShell {
  buildInputs = [ gnumake pyenv glibcLocales ];
  LOCALE_ARCHIVE_2_27 = "${glibcLocales}/lib/locale/locale-archive";
  LANG = "en_US.UTF-8";
}

