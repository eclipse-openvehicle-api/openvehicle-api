#include "includes.h"
#include "../../../sdv_executables/sdv_control/list_elements.h"


/*
* The following tests are needed: (- not implemented; + implemented)
* - install complex service - not running
* - update complex service - not running
* - uninstall complex service - not running
* - install complex service - running after installation
* - update complex service - running, stop, running after installation
* - uninstall complex service - running, stop, uninstall
* - install utility - not running
* - install utility - running
* - update utility - running
* - uninstall utility - not running
* - uninstall utility - running
* - install device - visible after restart
* - uninstall device - visible after restart
* - install basic service - visible after restart
* - uninstall basic service - visible after restart
* - install system service - visible after restart
* - uninstall system service - visible after restart
* - install complex service with dependencies of existing components - install okay, start okay
* - install complex service with dependencies of non-existing components - install okay, start not okay
* - uninstall complex service being dependent on for other complex service - uninstall okay, other component doesn't start
* - uninstall complex service being dependent on for other non-service component (e.g. utility) - uninstall after restart
* - uninstall complex service with modules having other components - uninstall after restart
* - update complex service with modules having other components - update fails
*/