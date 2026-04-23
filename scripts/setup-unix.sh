#!/usr/bin/env bash

set -e

packages="git cmake zip unzip build-essential pkg-config libgl1-mesa-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev libtool vulkan-validationlayers"

# Setup install_pkg
if [ "$(uname)" = "Linux" ]; then
	if [[ -f /etc/redhat-release ]]; then
		install_pkg="yum install -y"
		packages+=" autoconf"
	elif [[ -f /etc/fedora-release ]]; then
		install_pkg="dnf install -y"
		packages+=" autoconf"
	elif [[ -f /etc/arch-release ]]; then
		install_pkg="pacman -S --noconfirm"
		packages+=" autoconf automake"
	elif [[ -f /etc/gentoo-release ]]; then
		install_pkg="emerge"
		packages+=" autoconf"
	elif [[ -f /etc/SuSE-release ]]; then
		install_pkg="zypper install -n"
		packages+=" autoconf"
	elif [[ -f /etc/debian_version ]]; then
		install_pkg="apt-get install -y"
		packages+=" autoconf"
	elif [[ -f /etc/alpine-release ]]; then
		install_pkg="apk add --no-cache"
		packages+=" autoconf automake"
	fi
elif [ "$(uname)" = "Darwin" ]; then
	install_pkg="brew install"
else
	echo "Unsupported OS"
	exit 1
fi

sudo $install_pkg $packages

# Install dotnet
if ! which dotnet >/dev/null 2>&1; then
	echo "Installing dotnet..."
	if [ "$(uname)" = "Darwin" ]; then
		brew install dotnet
	else
		curl -fsSL https://dot.net/v1/dotnet-install.sh -o /tmp/dotnet-install.sh
		chmod +x /tmp/dotnet-install.sh
		/tmp/dotnet-install.sh --channel LTS
		rm /tmp/dotnet-install.sh

		# Make dotnet available system-wide
		export DOTNET_ROOT=$HOME/.dotnet
		export PATH=$PATH:$DOTNET_ROOT:$DOTNET_ROOT/tools
	fi
	echo "dotnet installed."
else
	echo "dotnet already installed."
fi

if ! which vcpkg >/dev/null; then
	echo "Installing vcpkg..."
	if [ ! -f "/opt/vcpkg" ]; then
		sudo git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg
	fi

	sudo /opt/vcpkg/bootstrap-vcpkg.sh
	sudo chown -R "$(id -u):$(id -g)" /opt/vcpkg

	echo "Create symlink from /usr/local/bin to /opt/vcpkg/vcpkg"
	sudo ln -sf /opt/vcpkg/vcpkg /usr/local/bin/vcpkg
	echo "vcpkg installed."
else
	echo "vcpkg already installed."
fi


# Determine vcpkg triplet from OS and CPU architecture
_arch=$(uname -m)
case "$_arch" in
	x86_64)            _vcpkg_arch="x64"   ;;
	aarch64 | arm64)   _vcpkg_arch="arm64" ;;
	armv7l)            _vcpkg_arch="arm"   ;;
	*)
		echo "Unsupported architecture: $_arch"
		exit 1
		;;
esac

if [ "$(uname)" = "Darwin" ]; then
	_vcpkg_triplet="${_vcpkg_arch}-osx"
else
	_vcpkg_triplet="${_vcpkg_arch}-linux"
fi

echo "Using vcpkg triplet: $_vcpkg_triplet"

pushd "${0%/*}/.." > /dev/null

cmake \
	-DCMAKE_TOOLCHAIN_FILE:STRING=/opt/vcpkg/scripts/buildsystems/vcpkg.cmake \
	-DVCPKG_TARGET_TRIPLET:STRING="${_vcpkg_triplet}" \
	.

popd > /dev/null
