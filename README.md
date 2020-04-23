# LXC playground

## Issue with AppArmor profile and no_new_privs bit in LXC

How to reproduce

```
$ vagrant up && vagrant ssh
$ cd /vagrant
$ mkdir build
$ cd build
$ cmake ..
$ make && sudo make install
$ # Reload apparmor profile
$ sudo systemctl restart apparmor
$ # This works, since main binary is running unconfined
$ sudo lxc-test-apparmor
$ # This fails, since main binary runs under profile, and no_new_privs forbids switching to profile other than subset
$ sudo aa-exec -p lxc-test-profile lxc-test-apparmor
```

See 

https://github.com/lxc/lxc/blob/master/src/lxc/attach.c#L788 (no_new_privs),

https://github.com/lxc/lxc/blob/master/src/lxc/attach.c#L777 (LSM)

See also: 

https://github.com/lxc/lxc/blob/master/src/lxc/start.c#L1236 (LSM),

https://github.com/lxc/lxc/blob/master/src/lxc/start.c#L1243 (no_new_privs)

In attach.c, no_new_privs is set BEFORE LSM label is set.
In start.c, no_new_privs is set AFTER LSM label is set.

First sight: these blocks should be switched in attach.c, i.e. AppArmor profile switch should go before setting no_new_privs flag

