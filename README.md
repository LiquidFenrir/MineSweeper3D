# MineSweeper3D

An open source homebrew recreation of minesweeper in C++ for the 3DS, with a twist!  
You're in a 3D world now, oh yeah! (note: this is NOT 3D minesweeper in the sense of a cubic grid. This is regular minesweeper but with a first person view)  
You have different board modes:

- Regular: the minesweeper you know and love
- Looping vertically: going above the top loops around to the bottom, and vice versa
- Looping horizontally: going far to the left loops around to the right, and vice versa
- Looping in both directions

And you can play against or with friends over the local connection! 
You all share the same map, but play it however you feel like.  
Available preset gamemodes are:

- Team competition: first team to complete the level or last team alive wins
  - Teams of 1 to 8 people, 2 to 8 teams
- Battle Royale: last man standing or first to complete the level wins
  - Teams of 1, 2 to 8 teams
- Co-op: everyone must cooperate to complete the level as fast as 
  - Team of 1 to 8 people, 1 team

## Controls

You can select which keys are used for revealing, which keys are for toggling flags, and which keys are for movement in the settings menu
Press START go back to the main menu while in game.

## License

This version of the game is licensed under the GPLv3.

## Credits

- `gfx/menus/room_coop.png` is licensed under the [CC-BY 4.0](https://creativecommons.org/licenses/by/4.0/deed.en) and made by Font Awesome Free 5.2.0 by @fontawesome - https://fontawesome.com
- `gfx/menus/room_team.png` and `gfx/menus/room_br.png` are made by [Freepik](https://www.freepik.com) from [www.flaticon.com](https://www.flaticon.com/)
- the images in the `gfx/ingame/sheet_3D_cursors.png` spritesheet are from [OpenGameArt.org](https://opengameart.org/) and licensed under the [CC0](https://creativecommons.org/publicdomain/)

Many thanks to the [libctru](https://github.com/smealum/ctrulib/), [citro3d](https://github.com/fincs/citro3d/), and [citro2d](https://github.com/devkitPro/citro2d/) maintainers and contributors for the amazing libraries.  
Even more thanks to fincs specifically for the 3ds examples of using citro3d, they were a lot of help!
