# Server configuration

This is a complete list of all supported settings for the server.ini
configuration file. See [example_server.ini](../example_server.ini) for a more
practical example.

The server.ini actually uses Plasma console commands, which are also used by
.fni files. Unless indicated otherwise, all values should be surrounded with
double quotes to avoid parsing issues with spaces or other symbols.

To use a different configuration file than the default server.ini, you can run
the client with the option `-ServerIni=path/to/other.ini`.

## Server.DispName

**Example:** `Server.DispName "Example Shard"`

A human-readable name for the shard. Optional.

This value is currently never displayed to the user, but it is used as an
identifier for the shard when saving login information. That is, two shards with
the same `Server.DispName` will share their saved login info with each other.

## Server.Status

**Example:** `Server.Status "https://moula.example.net/status"`

The full URL of the server status page to be displayed in the patcher and login
dialogs. Optional - if omitted, the default status message "Welcome to URU" is
displayed forever.

## Server.Signup

**Example:** `Server.Signup "https://moula.example.net/signup"`

The URL that is opened by the "Need an account?" button in the login window.
Optional - if omitted, the button is disabled.

## Server.Port

**Example:** `Server.Port 14617`

The default server port number, used where the server address doesn't contain an
explicit port number. Optional - if omitted, defaults to the standard MOUL
server port 14617.

The value is an integer and shouldn't be quoted.

## Server.(Type).Host

**Example:**

```
Server.Gate.Host "moula.example.net"
Server.File.Host "moula.example.net"
Server.Auth.Host "moula.example.net"
```

The addresses of the gatekeeper, file, and auth servers, respectively. Each
address may be a domain name or a plain IP addresses, optionally including a
port number. If no explicit port is specified, the default port from
`Server.Port` is used.

`Server.File.Host` and/or `Server.Auth.Host` are optional - if omitted, the
respective addresses are obtained dynamically from the gatekeeper server.
`Server.Gate.Host` is required, unless all other addresses are set explicitly.
There is no `Server.Game.Host` setting, because the game server address is
always obtained dynamically from the auth server.

For most shards, `Server.Gate.Host` and `Server.Auth.Host` should be set and
`Server.File.Host` should be left unset.

## Server.(Type).N/X

**Example:**

```
Server.Gate.N "kEso0EnSJNkgyQtVyUO8IGAzy9cVgtEZG3Dy/s0urdYgu+omUP0/3sQihN5EOJqqXQl2ahI7cifHptdDafekOA=="
Server.Gate.X "Pds9j7NYWE9o6WV6o88EhGczzxR/NpNimHn9yLq2X9xoAdiEH1x2R1w/O5nwWv3CPmVEPgm9IalIcr0CvZOy+Q=="
Server.Auth.N "tSNEa6OAIdfDby8p+lW8YOxqDZL1VUwVHPHbx01MuNxQ1Un8tlWYFi5mqzQPIZqjI3rX0YFLJatQUHr45jX6jQ=="
Server.Auth.X "GvO3GWDQaWnK+ZSJxTKCaEfOjWwJxMt85H9vvqC+nA84mw9hpqlezfyz86k5Nfo5BQ+dg5hivAcuK3CQC/+Isw=="
Server.Game.N "j1amo5e+ID/FhzgSEm2razSbZjhXEWEKGlTCZstbMU3kt3gMyANeR7mOw0MbRf5y7uV1Q4hhfCmRj3HrQhkXLQ=="
Server.Game.X "G+nYVfMDb/q8zT1qs7uy2j91jLmesUPq72TVLlUB7OXrWyP77ponIPlZDfSPu8EFVbndJWHONlOtI8AIxh3PIQ=="
```

The Diffie-Hellman *n* and *x* values for the gatekeeper, auth, and game server
connections, respectively. The values are packed 512-bit integers in big-endian
(OpenSSL) byte order, encoded as base-64. Note that this is opposite from the
little-endian byte order used in the pnNetBase .hpp files in Cyan's original
source.

Any pair of `N`/`X` settings may be omitted to disable encryption for that
server type. This is only recommended for testing purposes - public shards
should always have all six keys configured.

## Server.(Type).G

**Example:**

```
Server.Gate.G 4
Server.Auth.G 41
Server.Game.G 73
```

The Diffie-Hellman *g* values for the gatekeeper, auth, and game server
connections, respectively. Optional - if any of these settings is omitted, the
standard *g* value for that server type is used. Most shards use these standard
values, so there's usually no need to set these explicitly.

The values are integers and shouldn't be quoted.
