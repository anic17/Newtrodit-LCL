<!-- Change this to use a picture in the root directory. Not the bmp -->
<a href="https://github.com/anic17/Newtrodit-LCL"><img src="https://github.com/anic17/Newtrodit-LCL/blob/main/res/logo_transp.png" align="right" width="150" height="150" /></a>

#### [❔ About](https://github.com/anic17/Newtrodit-LCL#about) - [💡 Features](https://github.com/anic17/Newtrodit-LCL#features) - [📖 Requirements](https://github.com/anic17/Newtrodit-LCL#requirements) - [🔨 Build](https://github.com/anic17/Newtrodit-LCL#build)

# Newtrodit-LCL <br><a href="https://github.com/anic17/Newtrodit/stargazers">![newtrodit-stars](https://img.shields.io/github/stars/anic17/Newtrodit?color=yellow&style=flat-square)</a> <a href="https://github.com/anic17/Newtrodit/network/members">![newtrodit-forks](https://img.shields.io/github/forks/anic17/Newtrodit?style=flat-square)</a> <a href="https://www.gnu.org/licenses/gpl-3.0">![newtrodit-license](https://img.shields.io/github/license/anic17/Newtrodit?style=flat-square)</a> <a href="https://github.com/anic17/Newtrodit/issues">![newtrodit-issues](https://img.shields.io/github/issues/anic17/Newtrodit?style=flat-square)</a>

Official Newtrodit-LCL (Linux Compatibility Layer) repository, all code is under the GNU GPL v3.0 licence see [LICENSE](https://github.com/anic17/Newtrodit-LCL/blob/main/LICENSE) *or* [GPL 3.0](https://www.gnu.org/licenses/gpl-3.0.en.html) for more info!

###### Default Newtrodit theme
<img src="https://github.com/anic17/Newtrodit-LCL/blob/main/res/screenshot_main.png" width="521" height="300">

###### Dark line counting theme
<img src="https://github.com/anic17/Newtrodit-LCL/blob/main/res/screenshot_dark.png" width="521" height="300">

## Motives
&emsp;I once tried to make a clone of the old MS-DOS [EDIT.COM](https://en.wikipedia.org/wiki/MS-DOS_Editor) in batch. Though during the making of it I realized something. It would have been **really** complicated to use batch, and performance issues were starting to appear.

As I was quite new to C, creating Newtrodit was a challenge to see how good I could make an EDIT.COM clone with more features. The UI got changed plenty of times —it even had a blue background back then— until it became what it is now. I started to love coding Newtrodit, and that's why today I'm continuing to develop Newtrodit to this day.

# Features
- Fast and light
- Real time position of cursor
- Line counting
- Highly configurable
- Various string manipulation functions such as:
   - String finding (case sensitive and insensitive)
   - String replacing
   - ROT13 encoding
   - Uppercase/lowercase conversion
- Fully configurable syntax highlighting including custom rules
- Built-in manual
- Mouse support (Windows exclusive)
- Can edit files up to 5600 lines
- Some file utilities:
   - File compare
   - File locating

# Requirements
Newtrodit doesn't need any special requirements for Windows. It can run on Windows versions starting from Windows XP to Windows 11. 

When running Newtrodit on linux you must have any semi-modern terminal emulator, along with a shell that supports VT520, and DECSCUSR sequences. For some features, you also require a POSIX compilent system (which most linux distro's are). 

A list of tested Linux terminals on the three most popular shells can be found below 
| _Tested terminals_ | **BASH 5.1** | **ZSH 5.8** | **FISH 3.1** |
|--------------------|:------------:|:-----------:|:------------:|
| GNOME Terminal     |       ✓      |      ✓      |       ✓      |
| Terminator         |       ✓      |      ✓      |       ✓      |
| Konsole            |       ✓      |      ✓      |       ✓      |
| Guake              |       ✓      |      ✓      |       ✓      |
| XTERM              |       ✓      |      ✓      |       ✓      |
| RXVT               |       ✓      |      ✓      |       ✓      |
| Alacritty          |       ✓      |      ✓      |       ✓      |

*If your Terminal emulator wasn't listed don't worry*. Any terminal using the following backends are supported: xterm-color, xterm-16color, and xterm-256color. Other backends that fall under the XTerm specifications are: putty, konsole, Eterm, rxvt, gnome. 

Screen mode isn't fully supported and you may find features missing, and if you are using a dumb terminal Newtrodit will fail to run.

If you don't know what any of that stuff means, you don't have to worry about it. Newtrodit should run just fine on your terminal.

# Build
The easiest way to build Newtrodit is to use the `make` files.

Run [`make.bat`](make.bat) if you're Windows, or [`make.sh`](make.sh) if you're using a Linux system. 

# Contributing
Contributions are what make the open source community such an amazing place to learn, inspire, and create. Any contributions you make are **greatly appreciated**.

If you have a suggestion that would make this better, please fork the repo and create a pull request. You can also simply open an issue with the tag "enhancement".
Don't forget to give the project a star! Thanks again!

1. Fork the project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request
# Contact

You can join the Program Dream Discord server here and talk about everything related to Newtrodit.</br>
<a href="https://discord.gg/J628dBqQgb"><img src="https://img.shields.io/discord/728958932210679869?style=flat-square"></a>
<hr>

## Credits
Thanks to all the contributers who helped make Newtrodit possible! ❤️

<table>
  <tr>
    <td align="center"><a href="https://github.com/anic17"><img src="https://avatars.githubusercontent.com/u/58483910?v=4?s=100" width="100px;" /><br /><sub><b>anic17</b></sub></a><br /><a href="" title="Maintainer">:hammer:</a> <a href="" title="Code">:computer:</a></td>
    <td align="center"><a href="https://github.com/ZackeryRSmith"><img src="https://avatars.githubusercontent.com/u/72983221?v=4?s=100" width="100px;" alt=""/><br /><sub><b>ZackeryRSmith</b></sub></a><br /><a href="" title="Maintainer">:hammer:</a> <a href="" title="Code">:computer:</a></td>
    <td align="center"><a href="https://github.com/timlg07"><img src="https://avatars.githubusercontent.com/u/33633786?v=4?s=100" width="100px;" /><br /><sub><b>timlg07</b></sub></a><br /><a href="" title="Bug fixes">:bug:</a></td>
  </tr>
</table>

<hr>

###### Copyright &copy; 2021-2022 anic17 Software
