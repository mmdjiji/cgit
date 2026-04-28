# CGit

> This project is a fork of [alpine-cgit](https://github.com/joseluisq/alpine-cgit) and will be independently maintained going forward. The primary repository is hosted at [cnb.cool/mmdjiji/cgit](https://cnb.cool/mmdjiji/cgit) — please submit all issues and pull requests there. The GitHub repository is a read-only mirror.

The hyperfast web frontend for [Git](https://git-scm.com/) repositories on top of [Alpine](https://alpinelinux.org/) and [Nginx](https://nginx.org/). [CGit](https://git.zx2c4.com/cgit/about/) is a web interface ([cgi](https://tools.ietf.org/html/rfc3875)) for [Git](https://git-scm.com/) repositories, written in C.

<img src="./cgit.png" width="600">

## CGit features

- Basic repository browsing (logs, diffs, trees...).
- Caching of generated HTML.
- Cloneable URLs (implements dumb HTTP transport).
- Commit feeds (atom format).
- Discovery of Git repositories.
- On-the-fly archives for tags and commits.
- Plugin support e.g. syntax highlighting.
- Side-by-side diffs.
- Simple time/author statistics.
- Simple virtual hosting support (macro expansion).
- Understands GitWeb project-lists.
- Understands `gitweb.owner` in Git config files.
- Has an extensive filtering framework using scripts or a built-in Lua interpreter.
- Optional HTTP Basic Auth support.
- Git LFS online preview support.

## Usage

```sh
docker run -itd --name cgit -p 8787:80 -v $PWD/repos:/srv/git/ docker.cnb.cool/mmdjiji/cgit:latest
```

**Dockerfile**

```Dockerfile
FROM docker.cnb.cool/mmdjiji/cgit:latest
```

## Key container paths

- `/etc/cgitrc`: Default CGit configuration file.
- `/srv/git`: Default directory for Git repositories scanned by CGit.
- `/var/cache/cgit`: Default CGit caching directory of generated HTML.

Note that all these paths can be overwritten via [Bind Mounts](https://docs.docker.com/storage/bind-mounts/) or [Docker Volumes](https://docs.docker.com/storage/volumes/).

## Settings via environment variables

CGit Docker image can be configured via environment variables. This is the default behavior.

- `CGIT_TITLE`: Website title.
- `CGIT_DESC`: Website description.
- `CGIT_VROOT`: Virtual root directory.
- `CGIT_SECTION_FROM_STARTPATH`: How many path elements from each repo path to use as a default section name.
- `CGIT_MAX_REPO_COUNT`: Number of entries to list per page on the repository index page.

### Basic Authentication

HTTP Basic Auth can be enabled by setting both `BASIC_AUTH_USER` and `BASIC_AUTH_PASS`. If either is unset, authentication is disabled.

- `BASIC_AUTH_USER`: Username for HTTP Basic Auth.
- `BASIC_AUTH_PASS`: Password for HTTP Basic Auth.

This applies to both the web interface and git clone over HTTP:

```sh
git clone https://admin:12345678@your-host/repo-name
```

## Settings via custom configuration file

By default, this Docker image will use a template file located at [cgit.conf](./cgit.conf) which is replaced with the env settings (mentioned above) at start-up time.

However, if you want to use a custom `/etc/cgitrc` file then follow these steps:

1. Provide the env variable `USE_CUSTOM_CONFIG=true` to prevent using the default config file.
2. Provide the custom config file as a [Bind Mount](https://docs.docker.com/storage/bind-mounts/) or [Docker Volume](https://docs.docker.com/storage/volumes/). For example `--volume my-config-file:/etc/cgitrc`
3. Provide the `cache-root` option in your config file. For example `cache-root=/var/cache/cgit`
4. Provide the `scan-path` option in your config file. For example `scan-path=/srv/git`
5. Provide the repositories folder as a [Bind Mount](https://docs.docker.com/storage/bind-mounts/) or [Docker Volume](https://docs.docker.com/storage/volumes/). For example `--volume my-repos:/srv/git`

See [`cgitrc` man page](https://linux.die.net/man/5/cgitrc) for more detailed information.

## Contributions

This project is maintained at [cnb.cool/mmdjiji/cgit](https://cnb.cool/mmdjiji/cgit). The GitHub repository is a read-only mirror. Please submit all [Pull Requests](https://cnb.cool/mmdjiji/cgit/-/pulls) and [Issues](https://cnb.cool/mmdjiji/cgit/-/issues) on CNB.

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in current work by you, as defined in the Apache-2.0 license, shall be dual licensed as described below, without any additional terms or conditions.

## License

This work is primarily distributed under the terms of both the [MIT license](LICENSE-MIT) and the [Apache License (Version 2.0)](LICENSE-APACHE).

- © 2021-2026 [Jose Quintana](https://joseluisq.net)
- © 2026-present [JiJi](https://mmdjiji.com)
