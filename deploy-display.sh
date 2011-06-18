#!/bin/bash

rsync -avuz --delete ~/src/BioFame/BioDisplay-debug/BioDisplay.app bioscreen.local:src/BioFame
