#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/apparmor.h>

#include <lxc/lxccontainer.h>

int lxcWaitForPid(pid_t pid)
{
    int status, ret;

again:
    ret = waitpid(pid, &status, 0);
    if (ret == -1) {
        if (errno == EINTR)
            goto again;

        return -1;
    }

    if (ret != pid)
        goto again;

    return status;
}

void printCurrentAppArmorProfile()
{
    char *label, *mode;

    aa_getcon(&label, &mode);

    printf("%s (%s)\n",(label ? label : "null"), (mode ? mode : "null"));

    if (label)
        free(label); 
}

int containerFunc()
{
    printf("Current (CONTAINER) AppArmor profile (pid %d):\n", getpid());
    system("cat /proc/self/attr/apparmor/current");
    return 1;
}

int executeInContainer(void *arg)
{
    typedef int (*ExecFunction)();
    ExecFunction f = (ExecFunction)arg;
    return (*f)();
}

int runAppArmor()
{
    const char templateFilePath[] =  EXAMPLE_TEMPLATE_PATH;
    const char configFilePath[] = EXAMPLE_CONFIG_INSTALL_PATH;

    printf("LXC version: %s\n", lxc_get_version());

    struct lxc_log log = {
        .name    = "log",
        .lxcpath = NULL,
        .file    = NULL,
        .level   = "TRACE",
        .prefix  = "container",
        .quiet   = false,
    };

    printf("Current (HOST) AppArmor profile: \n");
    printCurrentAppArmorProfile();

    int ret = lxc_log_init(&log);
    if (ret < 0) {
        fprintf(stderr, "Failed to initialize LXC logging\n");
        return 1;
    }

    const char *containerID = "TestAppArmor";
    struct lxc_container *container = NULL;

    container = lxc_container_new(containerID, NULL);
    if (!container) {
        fprintf(stderr, "Error creating a new container\n");
        return 1;
    }

    if (container->is_defined(container)) {
        fprintf(stderr, "ContainerID '%s' is already in use\n", containerID);
        return 1;
    }

    printf("Successfully created container struct\n");

    if (!container->load_config(container, configFilePath)) {
        fprintf(stderr, "Error loading container config\n");
        return 1;
    }

    char *const createArgs[] = {NULL};
    if (!container->create(container, templateFilePath, NULL, NULL, 0, createArgs)) {
        fprintf(stderr, "Error creating container\n");
        return 1;
    }

    printf("Starting container\n");

    char* const args[] = { "env", "/bin/sleep", "100000000", NULL };
    pid_t pid, attachPid;
    lxc_attach_options_t options = LXC_ATTACH_OPTIONS_DEFAULT;
    int err = 1;

    if (!container->start(container, false, args)) {
        fprintf(stderr, "Error starting container\n");
        goto fin;
    }

    printf("Container started: %s\n", containerID);
    pid = container->init_pid(container);

    if (container->attach(container, &executeInContainer, (void*)&containerFunc, &options, &attachPid) < 0) {
        fprintf(stderr, "Failed to attach to container\n");
        goto fin;
    }

    printf("Attached PID: %d\n", attachPid);

    err = lxcWaitForPid(attachPid);
    if (err < 0) {
        fprintf(stderr, "failed to wait for pid %d\n", attachPid);
        goto fin;
    }

    err = 0;

fin:

    if (container) {
        if (!container->shutdown(container, 1)) {
            fprintf(stderr, "Failed to cleanly shutdown container %s forcing stop\n", containerID);

            if(!container->stop(container)) {
                fprintf(stderr, "Failed to force stop the container %s\n", containerID);
            }
        }

        if (!container->destroy(container))
            fprintf(stderr, "Failed to destroy container %s\n", containerID);

        lxc_container_put(container);
    }

    return err;
}

int main(int argc, char *argv[])
{
    return runAppArmor();
}
