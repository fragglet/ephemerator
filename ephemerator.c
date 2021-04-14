
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>

#define _GNU_SOURCE
#define __USE_GNU
#include <sys/socket.h>
#include <dlfcn.h>

static int (*orig_bind)(int socket, const struct sockaddr *addr,
                        socklen_t length);
static int low_port, high_port;
static int next_port;

static int init(void)
{
	char *eph_env;
	orig_bind = dlsym(RTLD_NEXT, "bind");
	if (orig_bind == NULL) {
		return 0;
	}

	eph_env = getenv("EPHEMERAL_PORTS");
	if (eph_env == NULL
	 || sscanf(eph_env, "%d %d", &low_port, &high_port) != 2) {
		fprintf(stderr,
		        "EPHEMERAL_PORTS not defined or wrong format.\n"
		        "You need to specify it with eg.\n"
		        "  export EPHEMERAL_PORTS=\"1000 2000\".\n");
		return 0;
	}

	next_port = low_port;
	return 1;
}

int bind(int socket, const struct sockaddr *addr, socklen_t length)
{
	struct sockaddr_in addr_in;
	int start_port, result;
	static int firsttime = 1;

	if (firsttime) {
		if (!init()) {
			errno = EADDRINUSE;
			return -1;
		}
		firsttime = 0;
	}

	// TODO: AF_INET6 support

	// We only care about the specific case where we're binding to a
	// TCP/UDP port and letting the OS pick an ephemeral port (sin_port=0)
	if (addr->sa_family != AF_INET
	 || length != sizeof(struct sockaddr_in)
	 || ((struct sockaddr_in *) addr)->sin_port != 0) {
		return orig_bind(socket, addr, length);
	}

	// Since `addr` is const, we make a copy that we can mutate.
	memcpy(&addr_in, addr, length);

	start_port = next_port;
	for (;;) {
		addr_in.sin_port = htons((unsigned short) next_port);
		result = orig_bind(socket, (struct sockaddr *) &addr_in,
		                   sizeof(struct sockaddr_in));

		// Update next_port for next call/next attempt.
		++next_port;
		if (next_port > high_port) {
			next_port = low_port;
		}

		if (result == 0) {
			return 0;
		}
		if (result != -1 || errno != EADDRINUSE) {
			break;
		}

		if (next_port == start_port) {
			// Looped all the way round and none are available.
			errno = EADDRINUSE;
			break;
		}
	}

	return -1;
}

