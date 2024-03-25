# Contributing
We are glad that you are interested in contributing to the Plasma Engine! We happily accept contributions, including:
- bug reports
- bug fixes
- feature requests
- new features
- art assets
- and much more!

Please take a moment to read these guidelines to ensure your contributions are accepted.

## Project Goals
The goal of the H-uru Plasma project is to improve on the open source release of the CyanWorlds.com Engine used in Myst Online: Uru Live. In the interest of doing so, we have identified these project goals:
- improving cross-platform and compiler compatibility
- removing dependencies on nonfree libraries
- fixing technical debt by utilizing modern language features in both C++ and Python
- improving accessibility for fan Age creation and testing
- improving testing coverage by adding unit tests and continuous integration
- improving the player experience
- avoiding nontrivial breaks in compatibility with the official **Myst Online: Uru Live (again)** game run by Cyan Worlds, Inc.

Information about specific tasks can be found on the [project roadmap](https://github.com/H-uru/Plasma/wiki/Roadmap).

Further, we have identified these non-goals:
- maintaining a game engine to be used outside of the context of Myst Online: Uru Live
- supporting legacy platforms, such as Windows 2000, Windows XP, Playstation 2, and XBox
- supporting exploit-based gameplay

These are tasks that run contrary to the project's priorities stated above, and as such are not likely to be accepted if submitted for inclusion. Any changes implementing these are best maintained on an independent fork.

## Getting Involved
Real-time discussion with team members and other contributors is an excellent way to begin contributing. We welcome feedback and discussion of proposed changes. Active maintainers can be found on the Guild of Writers IRC channel:
- Server: irc.guildofwriters.org:6667
- Channel: #writers

We also use the [Guild of Writers' forum](https://forum.guildofwriters.org/viewforum.php?f=114) for more permanent discussions. Further, many team members can also be found on the [OpenUru discord](https://discord.com/invite/tVknpHQ).

## Reporting Bugs and Requesting Features
We use GitHub's [issue tracker](https://github.com/H-uru/Plasma/issues) to list bugs and feature requests. Good bug reports tend to have:
- a summary or background
- steps to reproduce the bug, the more specific, the better!
- what you expect to happen
- what actually happens
- any other pertinent notes, such as why you think the issue is happening and any mitigation you attempted

## Submitting Code
We proudly develop using [GitHub Flow](https://guides.github.com/introduction/flow/index.html)! To propose changes to the repository:
- fork the repository and make your changes as described by GitHub flow
- be sure to follow the [Plasma coding style](https://github.com/H-uru/Plasma/wiki/Preferred-Code-Style)
- open a pull request and ensure that all test coverage and continuous integration passes
- document in the pull request body what you have changed and why

If you are working on a large changeset, we encourage you to open a draft pull request for commentary when you reach a minimally viable stage for feedback purposes. Pull requests may be merged when signed off on by at least one [maintainer](https://github.com/orgs/H-uru/people).
