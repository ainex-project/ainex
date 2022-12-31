<p align="center">
    <a href="https://github.com/ainex-project/ainex/tree/master/assets/icon">
        <img height="120px" src="https://raw.githubusercontent.com/ainex-project/ainex/master/assets/icon/128.png" />
    </a>
</p>

<h1 align="center">Ainex - Xbox 360 Emulator</h1>

Ainex is an experimental emulator of the Xbox 360. For more information, see the
[Ainex Wiki](http://ainex-project.github.io/wiki).

<!---
**Interested in supporting the core contributors?** Visit
[Ainex Project on Patreon](https://www.patreon.com/ainex_project).

Come chat with us about **emulator-related topics** on [Discord](https://discord.gg/Q9mxZf9).
For developer chat join `#dev` but stay on topic. Lurking is not only fine, but encouraged!
Please check the [FAQ](https://ainex-project.github.io/wiki/faq) page before asking questions.
We've got jobs/lives/etc, so don't expect instant answers.
--->

Discussing illegal activities will get you banned from discussions.

## Status

Platform | Status | Releases
-------- | ------ | --------
[Windows](https://github.com/ainex-project/ainex/actions/workflows/build-and-upload.yml) | ![status](https://img.shields.io/github/actions/workflow/status/ainex-project/ainex/build-and-upload.yml?label=%20&style=flat-square "Windows Build Status") | [Latest](https://github.com/ainex-project/ainex/releases/latest) ◦ [All](https://github.com/ainex-project/ainex/releases)

Many retail games run. Quite a few don't.
See the [compatibility list](https://github.com/ainex-project/game-compatibility/issues)
for currently tracked games and apps, and feel free to contribute your own updates,
screenshots, and information there following the [existing conventions](https://github.com/ainex-project/game-compatibility/blob/master/README.md).

## Disclaimer

The goal of this project is to experiment, research,
and implement emulation of the Xbox 360 hardware and operating system.
**It is not for enabling illegal activity**.
All information is obtained via reverse engineering of legally purchased devices
and games and information made public on the internet
(you'd be surprised what's indexed on Google...).

## [Building](https://ainex-project.github.io/wiki/development/building)

See [the wiki](https://ainex-project.github.io/wiki/development/building) for setup and information about the
`xb` script. When writing code, check the [style guide](https://ainex-project.github.io/wiki/development/style_guide)
and be sure to run clang-format!

## [Contributors Wanted!](https://ainex-project.github.io/wiki/development)

Have some spare time, know advanced C++, and want to write an emulator?
Contribute! There's a ton of work that needs to be done, a lot of which
is wide open greenfield fun.

**For general rules and guidelines please see [the wiki](https://ainex-project.github.io/wiki/development).**

Fixes and optimizations are always welcome! (Please!)
<!--- In addition to that there are some major work areas still untouched:

* Help work through [missing functionality/bugs in games](https://github.com/ainex-project/ainex/labels/compat)
* Reduce the size of Ainex's [huge log files](https://github.com/ainex-project/ainex/issues/1526)
* Skilled with Linux? A strong contributor is needed to [help with porting](https://github.com/ainex-project/ainex/labels/platform-linux)
--->

See more projects [good for contributors](https://github.com/ainex-project/ainex/labels/good%20first%20issue).
It's a good idea to <!-- ask on Discord and --> check the issues page before beginning work on something.

## [Wiki](https://ainex-project.github.io/wiki)

We believe actual documentation is very important.
The wiki hosts all the important information you might be looking for!

- [Frequently asked questions](https://ainex-project.github.io/wiki/faq)
- [Quickstart](https://ainex-project.github.io/wiki/faq/quickstart)
