let

  pkgs = import <nixpkgs> {};
  mypkgs = builtins.getFlake "git+https://codeberg.org/LittleYe233/packages.nix.git";
  libcron = mypkgs.packages.${builtins.currentSystem}.libcron;
  quickjs = mypkgs.packages.${builtins.currentSystem}.quickjspp;

in

pkgs.mkShell {
  nativeBuildInputs = with pkgs; [
    gcc
    gdb
    cmake # should add in this scope
    pkg-config
    clang-tools
    python313Packages.ninja
    uv
  ];

  buildInputs = [
    pkgs.curlFull
    pkgs.rapidjson
    pkgs.toml11
    pkgs.yaml-cpp
    pkgs.pcre2
    quickjs
    libcron
  ];

  shellHook = ''
    cd "${toString ./.}"
    echo "cwd: $(pwd)"
    if [ ! -d ".venv" ]; then
      # Use Python version from Alpine v3.23.3 Docker image
      uv venv --python 3.12 .venv
    fi
    source .venv/bin/activate
    uv pip install -r ./scripts/requirements.txt
  '';
}