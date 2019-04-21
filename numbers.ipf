.********************************************************
:userdoc.
:docprof toc=123.
:title.Zahlenspiel fÅr OS/2 PM 
.******************************************
:h1 res=100.  Idea of the game
:p.
:i1.Idea
The goal of this game is to combine 6 given numbers with +,*,-,/,  
such that a given aim results.  Each number may be used once at most.  
Division must give an integer result.
:p.
An example. Let
:xmp.
3 4 5 7 8 50 
:exmp.:p.
be given and
:xmp.
417 
:exmp.:p.
the aim. Then a solution would be
:xmp.
8*50+4*5-3 
:exmp.
:note.  You do not need to use all numbers.
:p.
The computer gives you random problems, which you should solve.
If you cannot find a solution, the computer can provide one.
:p.
Further information.
:ul compact.
:li. :link reftype=hd res=200.Seeking a Solution:elink.
:li. :link reftype=hd res=300.Entering a Problem:elink.
:li. :link reftype=hd res=400.About the Program:elink.
:eul.
.***********************************
:h1 res=200. Seeking a Solution
:p.
:i1.Mouse
:i1.Enter a Solution
:i1.Computer finds a Solution
To solve a problem, you can use one of the original 6 numbers or
a number you already computed. Click inside the number box or on the line with the equation. Used numbers are gray and cannot be selected. To clear your try, use the menu option.
:p.
To let the computer solve the problem, use the appropriate menu option.
:p.
After you found a solution, the computer will show you the amount of time you needed.
.***********************************
:h1 res=300. Entering own Problems
:i1.Entering a Problem
:p.
You might also define a problem. Choose the menu point and enter the
numbers and the aim. Numbers must be between 1 and 100. The aim must be between 1 and 1000.
.***********************************
:h1 res=400. About the Program
:p.
:i1.Author
:i1.Information
This program has been compiled with Borland-C++. You should get the source along with the executables. There is a help file easy.cpp, which contains classes to ease program development.
:p.
The primary goal was to learn about PM programming. However, I think this little game is nice.
:p.
Address of the author is
:xmp.
Dr. R. Grothmann
Ahornweg 5a
D-85117 Eitensheim
Germany

EMail: rene.grothmann@ku-eichstaett.de
CompuServe: 100333,3051
:exmp.
.***********************************
:euserdoc.
