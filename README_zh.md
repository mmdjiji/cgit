# CGit

> 本项目是 [alpine-cgit](https://github.com/joseluisq/alpine-cgit) 的分支，后续将独立维护。主仓库托管在 [cnb.cool/mmdjiji/cgit](https://cnb.cool/mmdjiji/cgit)，请在该平台提交 Issue 和 Pull Request。GitHub 仅为只读镜像。

基于 [Alpine](https://alpinelinux.org/) 和 [Nginx](https://nginx.org/) 的超快 [Git](https://git-scm.com/) 仓库 Web 前端。[CGit](https://git.zx2c4.com/cgit/about/) 是一个用 C 编写的 Git 仓库 Web 界面（[CGI](https://tools.ietf.org/html/rfc3875)）。

<img src="./cgit.png" width="600">

## 功能特性

- 基本仓库浏览（日志、差异、目录树等）
- 生成的 HTML 缓存
- 支持 HTTP 协议克隆
- Commit 订阅（Atom 格式）
- 自动发现 Git 仓库
- 按需生成 tag 和 commit 的归档文件
- 插件支持（如语法高亮）
- 并排差异对比
- 简单的时间/作者统计
- 简单的虚拟主机支持（宏展开）
- 兼容 GitWeb 项目列表
- 识别 Git 配置中的 `gitweb.owner`
- 支持脚本或内置 Lua 解释器的过滤框架
- 支持 HTTP Basic Auth 鉴权
- 支持 LFS 在线预览
- 丰富的主题支持

## 快速开始

```sh
docker run -itd --name cgit -p 8787:80 -v $PWD/repos:/srv/git/:ro docker.cnb.cool/mmdjiji/cgit:latest
```

**Dockerfile**

```Dockerfile
FROM docker.cnb.cool/mmdjiji/cgit:latest
```

## 容器关键路径

| 路径 | 说明 |
|------|------|
| `/etc/cgitrc` | CGit 配置文件 |
| `/srv/git` | Git 仓库扫描目录 |
| `/var/cache/cgit` | CGit HTML 缓存目录 |

以上路径均可通过 [Bind Mounts](https://docs.docker.com/storage/bind-mounts/) 或 [Docker Volumes](https://docs.docker.com/storage/volumes/) 覆盖。

## 环境变量配置

### CGit 基本配置

| 变量 | 说明 | 默认值 |
|------|------|--------|
| `CGIT_TITLE` | 网站标题 | `CGit` |
| `CGIT_DESC` | 网站描述 | `The hyperfast web frontend for Git repositories` |
| `CGIT_VROOT` | 虚拟根目录 | `/` |
| `CGIT_SECTION_FROM_STARTPATH` | 从仓库路径中提取多少级目录作为分区名 | `0` |
| `CGIT_MAX_REPO_COUNT` | 仓库索引页每页显示的条目数 | `50` |
| `CGIT_THEME` | 默认视觉主题 | `default` |

#### 可用主题

用户也可通过页面头部的下拉菜单在运行时切换主题，选择将保存在浏览器中。

| 值 | 说明 |
|------|------|
| `default` | 经典 CGit 外观 |
| `cgithub-auto` | GitHub 风格主题，跟随系统明暗模式 |
| `cgithub-light` | GitHub 风格主题，始终浅色 |
| `cgithub-dark` | GitHub 风格主题，始终深色 |

### HTTP Basic 认证

同时设置 `BASIC_AUTH_USER` 和 `BASIC_AUTH_PASS` 即可启用 HTTP Basic 认证，未设置则不启用。认证覆盖网页浏览和 Git 克隆。

| 变量 | 说明 |
|------|------|
| `BASIC_AUTH_USER` | 认证用户名 |
| `BASIC_AUTH_PASS` | 认证密码 |

启用后，Git 克隆时需要携带凭据：

```sh
git clone https://admin:12345678@your-host/repo-name
```

## 自定义配置文件

默认情况下，镜像使用 [cgit.conf](./cgit.conf) 模板文件，启动时替换上述环境变量。

如需使用自定义 `/etc/cgitrc`：

1. 设置环境变量 `USE_CUSTOM_CONFIG=true`
2. 挂载自定义配置文件，例如 `--volume my-config:/etc/cgitrc`
3. 配置文件中需包含 `cache-root=/var/cache/cgit`
4. 配置文件中需包含 `scan-path=/srv/git`
5. 挂载仓库目录，例如 `--volume my-repos:/srv/git`

详细配置项请参阅 [`cgitrc` man page](https://linux.die.net/man/5/cgitrc)。

## 贡献

本项目托管在 [cnb.cool/mmdjiji/cgit](https://cnb.cool/mmdjiji/cgit)，GitHub 仅为只读镜像。请在 CNB 上提交 [Pull Request](https://cnb.cool/mmdjiji/cgit/-/pulls) 或创建 [Issue](https://cnb.cool/mmdjiji/cgit/-/issues)。

除非您明确声明另有规定，否则您有意提交以纳入当前作品的任何贡献（如 Apache-2.0 许可证中所定义）将按如下所述进行双重许可，且不附加任何条款或条件。

## 许可证

本项目主要使用 [MIT](LICENSE-MIT) 和 [Apache 2.0](LICENSE-APACHE) 双重许可证分发。

- © 2021-2026 [Jose Quintana](https://joseluisq.net)
- © 2026-present [JiJi](https://mmdjiji.com)
