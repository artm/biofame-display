# 15 minutes of Biometric Fame display

## FaceTemplate::Ptr
    to keep multiple collections of pointers to face templates and delete templates when all pointers are removed
    - at the moment this is broken and FaceTemplate::Ptr is defined as FacePointer*

## TODO lists

[ ] matching order
    [ ] first - new images in reverse chronological order
    [ ] next - originals in no particular order

[ ] progress report when loading database...
    [ ] blink at start up - while reading the directory
[ ] if can't make template for DB image - rename it to xxx.jpg.broken
[ ] check if other image types are present

[ ] Limit the loaded database to:
    [ ] all originals
    [ ] N freshest news (e.g. twice as much)

[ ] crop database images (when?)

[ ] make crop show less black borders (check if crop hight is larger then orig image)
