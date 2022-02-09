{
  python3Packages
} :
with python3Packages;
buildPythonApplication {
  name = "pygments-tab";
  src = ./.;
  propagatedBuildInputs = [ pygments ];
}
