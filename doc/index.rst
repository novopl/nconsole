.. nconsole documentation master file, created by
   sphinx-quickstart on Wed Aug 31 14:46:42 2011.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

nconsole documentation
=======================

Console functionality is split into two basic parts. The console itself
which manages all of the console functionality and the console output.
Console uses nlogger as its output and defines it's own message type. To
handle the console messages you have to implement nlogger's ``LogOut``
interface and attach it to logger with message filter accepting only
:cpp:ar:`kConsoleMsg`.

Contents:

.. toctree::
   :maxdepth: 2

   cheatsheet
   reference

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

