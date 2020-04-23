
Vagrant.configure(2) do |config|
    # Setup VirtualBox Provider
    config.vm.box = "bento/ubuntu-19.10"

    config.vm.box_check_update = false
    config.vm.provider "virtualbox" do |vb|
        vb.gui = false
    end

    # Make sure we have a fresh list from the package server
    config.vm.provision "shell", inline: "sudo apt-get update && sudo apt-get upgrade -y"


    config.vm.provision "shell", inline: "sudo apt-get install -y build-essential automake libtool cmake pkg-config gdb valgrind apparmor apparmor-utils libapparmor1 libapparmor-dev seccomp libseccomp-dev"

    # Run shell scripts
    config.vm.provision "shell", inline: "sudo apt-get remove -fy lxc lxc-dev liblxc-dev liblxc1 liblxc-common lxc1 lxc-utils lxc-templates"
    config.vm.provision "shell", inline: "rm -rf lxc || true"
    config.vm.provision "shell", inline: "git clone https://github.com/lxc/lxc && cd lxc && git checkout lxc-3.1.0 && ./autogen.sh && ./configure --prefix=/usr --sysconfdir=/etc --enable-apparmor --enable-seccomp && make && sudo make install"
    config.vm.provision "shell", inline: "systemctl restart apparmor"
end

