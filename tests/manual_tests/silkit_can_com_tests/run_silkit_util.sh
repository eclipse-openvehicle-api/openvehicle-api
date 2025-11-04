#!/bin/bash

# Run sil-kit-registry in a new terminal window
gnome-terminal -- bash -c "./../../_deps/silkit-src/SilKit/bin/sil-kit-registry; exec bash"

# Run sil-kit-system-controller with arguments in another new terminal window
gnome-terminal -- bash -c "./../../_deps/silkit-src/SilKit/bin/sil-kit-system-controller can_reader can_writer; exec bash"