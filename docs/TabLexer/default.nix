{
  buildPythonPackage
, pygments
, setuptools
} :
buildPythonPackage {
  name = "pygments-tab";
  src = ./.;
  propagatedBuildInputs = [ setuptools pygments ];
}
