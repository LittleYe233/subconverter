#!/bin/bash
set -xe

apk add gcc g++ build-base linux-headers cmake ninja-is-really-ninja \
    autoconf automake libtool python3 mbedtls-dev mbedtls-static \
    zlib-dev rapidjson-dev zlib-static pcre2-dev

git clone https://github.com/curl/curl --depth=1 --branch curl-8_6_0
cd curl
cmake -DCURL_USE_MBEDTLS=ON -DHTTP_ONLY=ON -DBUILD_TESTING=OFF -DBUILD_SHARED_LIBS=OFF -DCMAKE_USE_LIBSSH2=OFF -DBUILD_CURL_EXE=OFF -G Ninja . > /dev/null
ninja install > /dev/null
cd ..

git clone https://github.com/jbeder/yaml-cpp --depth=1
cd yaml-cpp
cmake -DCMAKE_BUILD_TYPE=Release -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF -G Ninja . > /dev/null
ninja install > /dev/null
cd ..

git clone https://github.com/ftk/quickjspp
cd quickjspp
# Should use this version now, or it will not compile
git checkout 0c00c48
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -G Ninja . > /dev/null
ninja install > /dev/null
cd ..

git clone https://github.com/PerMalmberg/libcron --depth=1
cd libcron
git submodule update --init
# Remove test target. Refer to AUR package
sed -i '/add_subdirectory(test)/d' CMakeLists.txt
sed -i '/add_dependencies(cron_test libcron)/d' CMakeLists.txt
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja . > /dev/null
ninja install > /dev/null
cd ..

git clone https://github.com/ToruNiina/toml11 --branch="v4.3.0" --depth=1
cd toml11
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_STANDARD=11 -G Ninja . > /dev/null
ninja install > /dev/null
cd ..

export PKG_CONFIG_PATH=/usr/lib64/pkgconfig
cmake -DCMAKE_BUILD_TYPE=Release -G Ninja . > /dev/null
ninja
mv subconverter base/

python3 -m venv .venv
### ENTER Python virtual env
source .venv/bin/activate
pip install -r scripts/requirements.txt
python3 scripts/update_rules.py -c scripts/rules_config.conf
deactivate
### LEAVE Python virtual env

cd base
chmod +rx subconverter
chmod +r ./*
cd ..
mv base subconverter
