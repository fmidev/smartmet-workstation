#!/bin/bash

# Creates smartmet-server development environment on current directory.

# 1. Let's install required stuff on Linux:
sudo dnf -y install 'dnf-command(config-manager)'
sudo dnf config-manager --set-enabled crb
sudo dnf -y install https://dl.fedoraproject.org/pub/epel/epel-release-latest-9.noarch.rpm
sudo dnf config-manager --setopt="epel.exclude=eccodes*" --save
sudo dnf -y install wget
sudo dnf -y install rpm-build
sudo dnf -y install yum-utils
sudo dnf -y install dnf-plugins-core
sudo dnf config-manager --setopt=http_caching=packages --save
# Echo "Previous command probably didn't work, but it's not that necessary?"
sudo dnf -y install https://download.fmi.fi/smartmet-open/rhel/9/x86_64/smartmet-open-release-latest-9.noarch.rpm
sudo dnf -y install https://download.fmi.fi/smartmet-open/rhel/9/x86_64/smartmet-open-beta-latest-9.noarch.rpm
sudo dnf config-manager --enable smartmet-open-beta
sudo dnf -y install clang
sudo dnf -y install ccache
sudo dnf -y install createrepo_c
sudo dnf -y install git
sudo dnf -y install make
sudo dnf -y install sudo
sudo dnf -y install rpmlint
sudo dnf -y install https://download.postgresql.org/pub/repos/yum/reporpms/EL-9-x86_64/pgdg-redhat-repo-latest.noarch.rpm
sudo dnf config-manager --disable "pgdg*"
sudo dnf -y install ImageMagick
sudo dnf -y install smartmet-engine-grid-test smartmet-library-spine-plugin-test smartmet-test-db smartmet-test-data

# 2. Private side library installation:
sudo dnf -y install https://download.fmi.fi/smartmet-private/rhel/9/x86_64/smartmet-private-release-latest-9.noarch.rpm
sudo dnf -y install https://download.fmi.fi/smartmet-private/rhel/9/x86_64/smartmet-private-release-latest.noarch.rpm
sudo dnf -y install https://download.fmi.fi/fmiforge/rhel/9/x86_64/fmiforge-release-latest.noarch.rpm

# 3. Let's install some smartmet related stuff
sudo dnf install -x "*q3*" "smartmet-plugin*" "*delfoi*"
sudo debuginfo-install $(rpm -qa | grep smartmet)

# 4. gcc and related tools installation (is needed?)
sudo dnf -y install gcc-c++ gdb valgrind perf

# 5. Clone FMI modules from github repositories (reading structures from fmi_modules.txt file)

file="fmi_modules.txt"
declare -a github_module_names
declare -a local_module_names
declare -a local_relative_base_directory

while IFS=' ' read -r str1 str2 str3 _rest; do
    # Skip comment lines and empty lines
    if [[ $str1 != "#"* && -n $str1 ]]; then
        github_module_names+=("$str1")
        local_module_names+=("$str2")
        local_relative_base_directory+=("$str3")
    fi
done < "$file"

# Loop through modules and do git clone and needed updates
for index in "${!local_relative_base_directory[@]}"; do
    local_module_directory="${local_relative_base_directory[$index]}/${local_module_names[$index]}"
	
	git clone https://github.com/fmidev/${github_module_names[$index]} ${local_module_directory}
	# Some modules need dependency updates before they can be build
	sudo yum-builddep *spec
done

# 6. In Linux home directory there should be .gitconfig file, put this kind of information in the end
#[user]
#    name = Marko Pietarinen
#    email = marko.pietarinen@fmi.fi
#[push]
#    default = tracking
#[credential]
#    helper = store

# 7. Some server configurations are cloned
# 7.1. /smartmet/git/smartmetd-dev.git
# - From io.weatherproof.fi
# - Changed couple of files so that light version of server starts
# 7.2. WMS layers configurations
# git clone https://github.com/fmidev/smartmet-plugin-wms-conf.git wms-conf
# Those are stored in home directory
# 7.3. From io.weatherproof.fi:
# git clone /smartmet/git/smartmetd-clients-grid.git 
# - Contains clients-cluster configurations. On main level there is backend.conf, from where you can disable options.
