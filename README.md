About
-----

With this program you can get times of stand-start acceleration runs over *n* meters with an optional *m* meter rollout.

Input
-----
A *.csv* file generated from racebox session (custom setting with any time format accepted, different speed settings however have not been tested).

Output
------
`[time in the format specified in export of racebox csv] 0-[n]m time [if rollout is enabled: (with [m]m rollout)]: 00.000 s`

How to run it?
----------
Run the project in Microsoft Visual Studio, upload your source *.csv* file into the src/ folder and copy the file name into the import template in *.cpp* file. Then you can run program in your standard debugger.
