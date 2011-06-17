# 15 minutes of Biometric Fame display

## FaceTemplate::Ptr
    to keep multiple collections of pointers to face templates and delete templates when all pointers are removed
    - at the moment this is broken and FaceTemplate::Ptr is defined as FacePointer*

## TODO lists

[?] watch without help from OS
    - started to work suddenly
    [ ] test with the robot / adhoc network
    [ ] share Faces to guests
[ ] on the other side: upload to tmp/bla.jpg then mv to incoming/bla.jpg

[-] matching order
    - doesn't matter: the whole database is matched in under a second

[ ] progress report when loading database...
    [ ] blink at start up - while reading the directory
[ ] if can't make template for DB image - rename it to xxx.jpg.broken
[ ] check if other image types are present

[ ] crop database images (when?)

[ ] make crop show less black borders (check if crop hight is larger then orig image)

[ ] animation

[ ] daily maintainance script (on startup):
    [ ] log rotate, compress old logs
    [ ] probably kill the oldest pictures / templates in Faces/new
    [ ] also for the robot

