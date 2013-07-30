#!/bin/bash

rsync --recursive --inplace --times --delete --partial --compress --progress . androsov@cmspixel.pi.infn.it:pixel/workspace
