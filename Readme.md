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

[ ] !!! check if new templates are freed after compression !!!

[x] if can't make template for DB image - rename it to xxx.jpg.broken
[x] more filetypes: .gif, .png

[ ] crop database images (when?)
    [ ] the problem with cropping is that we show them in main thread, but the extractor "belongs" in biometric thread.
    we could assume, that by the time cropping is requested the extractor isn't used by the biometrics, but eventually this
    ass-umption would bite us in the ass. The simplest is to crop in biometric thread and communicate the cropped faces via
    signal.

[ ] make crop show less black borders (check if crop hight is larger then orig image)

[ ] animation

[ ] daily maintainance script (on startup):
    [ ] log rotate, compress old logs
    [ ] probably kill the oldest pictures / templates in Faces/new
    [ ] also for the robot

