#!/usr/bin/env bash

packages="git cmake zip unzip build-essential pkg-config libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libtool "

# Setup install_pkg
if [ "$(uname)" = "Linux" ]; then
	if [[ -f /etc/redhat-release ]]; then
		install_pkg="yum install -y"
		packages+="autoconf"
	elif [[ -f /etc/fedora-release ]]; then
		install_pkg="dnf install -y"
		packages+="autoconf"
	elif [[ -f /etc/arch-release ]]; then
		pkg_install="pacman -S --noconfirm"
		packages+="autoconf automake"
	elif [[ -f /etc/gentoo-release ]]; then
		install_pkg="emerge"
		packages+="autoconf"
	elif [[ -f /etc/SuSE-release ]]; then
		install_pkg="zypper install -n"
		packages+="autoconf"
	elif [[ -f /etc/debian_version ]]; then
		install_pkg="apt-get install -y"
		packages+="autoconf"
	elif [[ -f /etc/alpine-release ]]; then
		install_pkg="apk add --no-cache"
		packages+="autoconf"
	elif [[ -f /etc/alpine-release ]]; then
		install_pkg="apk add --no-cache"
		packages+="autoconf automake"
	fi
elif [ "$os" = "Darwin" ]; then
	install_pkg="brew"
else
	echo "Unsupported OS"
	exit 1
fi

sudo $install_pkg $packages

# Install vcpkg
if ! which vcpkg >/dev/null; then
	sudo git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg
	# - Make the directory
	sudo mkdir -p /opt/vcpkg
	# - Bootstrap
	sudo /opt/vcpkg/bootstrap-vcpkg.sh
	# - Create symlink from /usr/local/bin to /opt/
	sudo ln -sf /opt/vcpkg/vcpkg /usr/local/bin/vcpkg
fi

pushd "${0%/*}/.." > /dev/null
sudo cmake -DCMAKE_TOOLCHAIN_FILE:STRING=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake ./CMakeLists.txt
popd > /dev/null
