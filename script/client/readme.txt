Setup for auto starting startkladde:

Starting:
- /etc/init.d/startkladde start calls the startkladde-up script as user
  startkladde
- startkladde-up starts the X server and keeps restarting it if it dies
- the autostart mechanism of the window manager calls startkladde_wrapper
- startkladde_wrapper calls startkladde and restarts it if it dies. If the
  program dies with magic code 69, it calls sudo halt instead.

Stopping:
- /etc/init.d/startkladde stop calls the startkladde-down script as user
  startkladde
- startkladde-down kills the startkladde-up script (so the X server will not be
  restarted) and the X server

Restarting:
- /etc/init.d/startkladde restart calls itself once with stop and once with
  start
- /etc/init.d/startkladde reload kills the startkladde process which will be
  restarted by startkladde_wrapper

