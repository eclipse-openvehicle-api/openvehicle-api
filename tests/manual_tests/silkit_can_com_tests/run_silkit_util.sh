# /********************************************************************************
#  Copyright (c) 2025-2026 ZF Friedrichshafen AG
# 
#  This program and the accompanying materials are made available under the 
#  terms of the Apache License Version 2.0 which is available at
#  https://www.apache.org/licenses/LICENSE-2.0
# 
#  SPDX-License-Identifier: Apache-2.0 
# ********************************************************************************/
# 
# 
#!/bin/bash

# Run sil-kit-registry in a new terminal window
gnome-terminal -- bash -c "./../../_deps/silkit-src/SilKit/bin/sil-kit-registry; exec bash"

# Run sil-kit-system-controller with arguments in another new terminal window
gnome-terminal -- bash -c "./../../_deps/silkit-src/SilKit/bin/sil-kit-system-controller can_reader can_writer; exec bash"