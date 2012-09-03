

Cheat sheet
############

Initialization and cleanup
===========================

Since the console is not a singleton and does not require any
global system, we just need to create the ``Console`` object and
delete it after using. The console has it's own message type::
  
  Console *console = new Console();

  // Use console

  delete console;



