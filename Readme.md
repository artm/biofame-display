# 15 minutes of Biometric Fame display

## FaceTemplate::Ptr
    to keep multiple collections of pointers to face templates and delete templates when all pointers are removed
    - at the moment this is broken and FaceTemplate::Ptr is defined as FacePointer*

## TODO lists

[?] watch without help from OS
    - started to work suddenly
    [ ] test with the robot / adhoc network
    [ ] share Faces to guests
[ ] on the robot: upload to tmp/bla.jpg then mv to incoming/bla.jpg

[-] matching order
    - doesn't matter: the whole database is matched in under a second

[ ] animation

[ ] daily maintainance script (on startup):
    [ ] log rotate, compress old logs
    [ ] probably kill the oldest pictures / templates in Faces/new
    [ ] also for the robot

[ ] clear small faces when new ancestry is loaded
    - part of animation

