<div class="newtrodit-md">




# Newtrodit
Simple console text editor written in C. 

<a href="https://github.com/anic17/Newtrodit/stargazers">![newtrodit-stars](https://img.shields.io/github/stars/anic17/Newtrodit?color=yellow)</a>
<a href="https://github.com/anic17/Newtrodit/network/members">![newtrodit-forks](https://img.shields.io/github/forks/anic17/Newtrodit)</a>
<a href="https://www.gnu.org/licenses/gpl-3.0">![newtrodit-license](https://img.shields.io/github/license/anic17/Newtrodit)</a>
<a href="https://github.com/anic17/Newtrodit/issues">![newtrodit-issues](https://img.shields.io/github/issues/anic17/Newtrodit)</a>

<a href="https://anic17.github.io/Newtrodit/"><img src="https://user-images.githubusercontent.com/58483910/144676737-67ae58a7-d085-4c1c-920e-b61a39cc09a5.png"></img></a>


# Features

- Easy to use
- Fast and light
- Real time position of cursor
- Line counting
- Various string manipulation functions such as:
  - String finding
  - String replacing
  - ROT13 encoding
  - Uppercase/lowercase conversion
- Fully configurable syntax highlighting including custom rules

## More features to come

- Maybe an hex editor?

## Small history

### Why I started to code Newtrodit?

It was at a day when I tried to make a clone of the old MS-DOS EDIT.COM in batch, but in the making I realized it would be really complicated to make in batch and performance issues were starting to appear.  
As I was very new to C, it was a challenge to see how good I could recode it, but with more features.  
The UI got changed a lot of times, before it had blue background, and it was a lot different.  
I started to love coding it, and that's why today I'm continuing to develop Newtrodit

## Project building

Compiling and running Newtrodit doesn't require any external libraries apart from the WinAPI simple libraries. Build Newtrodit with the following command line:  
`tcc newtrodit.c -o newtrodit.exe -luser32`  

Or if you prefer using GCC:  
`gcc newtrodit.c -o newtrodit.exe -luser32 -O2`

# Compatibility

  Newtrodit is compatible with OSes starting from Windows XP, but you can enable a small support for Windows 95/98/ME by changing the `_NEWTRODIT_OLD_SUPPORT` constant to **1** in [`newtrodit_core.h`](../main/newtrodit_core.h).  However, this isn't recommended for regular builds, as the compiled executable will lack some features such as UTF-8 file reading.

## Bug reports

To report a bug, feel free to create an issue explaining the bug and the way to get it. Contribution is highly appreciated as Newtrodit is still on beta, so there's a bunch of bugs needing to be fixed.

## Contributing

If you want to contribute to Newtrodit, fork the project and create a pull request with the changes you want to do, describing a bit the changes you did.


## Manual
To get information about the usage of Newtrodit, press F1 at Newtrodit or run `newtrodit --help`  
It shows all the information you need to know to use Newtrodit. If you have any questions, contact me on <a href="https://discord.gg/J628dBqQgb" style="text-decoration: none">Discord</a>.

## Contact

Join the Newtrodit development channel!  
<a href="https://discord.gg/J628dBqQgb"><img src="https://img.shields.io/discord/728958932210679869"></a>


## Release

Newtrodit will be released in a short time, but you can compile using <a href="https://bellard.org/tcc/" style="text-decoration: none">TCC</a> and running the script [`make.bat`](../main/make.bat)
Or you can run the binaries inside the [`bin`](../main/bin/) directory.

#### Copyright &copy; 2021-2022 anic17 Software
</div>

<!-- 
View counter 
-->
<img src="https://hits.seeyoufarm.com/api/count/incr/badge.svg?url=https%3A%2F%2Fgithub.com%2Fanic17%2FNewtrodit&count_bg=%23FFFFFF&title_bg=%23FFFFFF&icon=&icon_color=%23FFFFFF&title=hits&edge_flat=false" style="display:none" height=0 width=0>

