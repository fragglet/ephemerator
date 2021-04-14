Ephemerator allows control of the ephemeral ports used by an application.

It works by using glibc's `LD_PRELOAD` functionality to override the `bind`
API function. When assigning an ephemeral port (by binding to port 0), the
alternate version of `bind` instead tries binding to ports within a
preconfigured range.

The potential use of this is pretty niche. It's only been tested with UDP
sockets. In my case, I wrote this to run a Quake server, to ensure that
the UDP ports ephemerally assigned to individual users would be in a
particular range to be able to pass a firewall.

