# MineSweeper3D

An open source homebrew recreation of minesweeper in C++ for the 3DS, with a twist!  
You're in a 3D world now, oh yeah! (note: this is NOT 3D minesweeper in the sense of a cubic grid. This is regular minesweeper but with a first person view)  

## Controls

Press START at any time to exit.  
Press SELECT at any time to toggle the settings menu (look/move bindings, y-axis inversion, and look sensitivity).  

At first, you can use X to edit the width of the level, Y to edit the height of the level, and B to edit the percentage of bombs.  
A will select the play button, and another A press will launch the game!

Look around with the D-Pad/Circle Pad, and move with ABXY in their respective direction!  
You can 'R'eveal a square with the R shoulder button (this will generate the entire level the first time you do that on any level, which can freeze for a few frames)
You can p'L'ant a f'L'ag with the L shoulder button, after you've revealed once. This will prevent revealing bombs and losing!  

After losing or winning, pressing L or R will bring you back to the level edition screen, but before that you can still move around.

## License

This version of the game is licensed under the GPLv3.

## Credits

Many thanks to the [libctru](https://github.com/smealum/ctrulib/), [citro3d](https://github.com/fincs/citro3d/), and [citro2d](https://github.com/devkitPro/citro2d/) maintainers and contributors for the amazing libraries.  
Even more thanks to fincs specifically for the 3ds examples of using citro3d, they were a lot of help!
