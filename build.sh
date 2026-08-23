#!/usr/bin/env bash

# Quick help message helper
print_help() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  --x11        Build for KWin X11 instead of the Wayland default"
    echo "  --kinoite    Build an RPM package for Fedora Kinoite"
    echo "  -h | --help  Displays this message"
}

# Specific variables for X11 and Fedora Kinoite
X11_FLAG=""
KINOITE_MODE=0

# Parsing arguments
for arg in "$@"; do
    case "$arg" in
        --x11)
            X11_FLAG="-DBBDX_X11=ON"
            ;;
        --kinoite)
            KINOITE_MODE=1
            ;;
        -h|--help)
            print_help
            exit 0
            ;;
        *)
            echo "Unknown parameter passed: $arg"
            echo ""
            print_help
            exit 1
            ;;
    esac
done

# Common building part of both regular and fedora kinoite versions
rm -fr build
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr $X11_FLAG
make -j$(nproc)

# Handle installation or packaging based on the mode
if [[ $KINOITE_MODE -eq 1 ]]; then
    cpack -V -G RPM

    echo "To finish installation on Fedora Kinoite, exit your container and run:"
    echo "sudo rpm-ostree install kwin-effects-better-blur-dx-wobbly/build/kwin-better-blur-dx-wobbly-api.rpm"
else
    sudo make install
fi
