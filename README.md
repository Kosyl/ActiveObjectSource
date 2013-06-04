ActiveObjectSource
==================
Kompilacja:

>scons

...aplikacja testowa w trybie release (SimpleApp.exe)

>scons -Q debug=1

...aplikacja testowa w trybie debug (SimpleAppDeb.exe)

>scons -Q tests=1

...kompiluje test suite (Tests.exe)

>scons -Q tests=1 debug=1

...kompiluje test suite w trybie gadatliwym. (TestsDeb.exe)

Projekt testowano na platformach: Linuks, g++ (4.63) i Windows, Visual C++ (2010). Korzystano 
z bibliotek boost 1_53_0:
- hpp:boost/function.hpp, boost/thread.hpp, boost/thread/mutex.hpp, boost/thread/recursive_mutex.hpp, 
  boost/thread/condition_variable.hpp, boost/smart_ptr/shared_ptr.hpp, boost/noncopyable.hpp, 
  boost/any.hpp,boost/signal.hpp, boost/exception_ptr.hpp, boost/ref.hpp, boost/weak_ptr.hpp,
  boost/shared_ptr.hpp, boost/bind.hpp, boost/date_time/gregorian/gregorian.hpp,
  boost/date_time/posix_time/posix_time.hpp, boost/thread.hpp, boost/test/minimal.hpp
  
- lib: boost_thread, boost_signals, boost_unit_test_framework

Warunki poprawnej kompilacji:
- ustawienie prawidlowo sciezek do bibliotek .hpp: (na ogol: CCPATH na '.../boost_1_53_0')
- ustawienie prawidlowo sciezek do bibliotek .lib: (na ogol: LIBBATH na '.../boost_1_53_0/stage/lib')
W miejsce ... nalezy wpisac lokalizacje katalogu boost_1_53_0.
