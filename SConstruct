import platform, shutil

e = Environment()

if(platform.system() == "Linux"):
   e.Append( CPPFLAGS = '-Wall -pedantic -pthread -Wno-long-long' )
   e.Append( LINKFLAGS = '-Wall -pthread' )
   e.Append( CPPPATH = ['/usr/include/python2.7'] )
   e.Append( LIBPATH = ['/usr/lib/python2.7'] )
   e.Append( LIBS = [ 'boost_python' ] )

   target = 'Main.so'
elif(platform.system() == "Windows"):
   e.Append( CPPFLAGS = ' /EHsc /D "WIN32" /D "_WIN32_WINNT#0x501" /D "_CONSOLE" /W4 /MD' )
   e.Append( CPPPATH = [ 'c:/Program Files/Boost/boost_1_53_0', 'C:/Python27/include' ] )
   e.Append( LIBPATH = [ 'C:/Program Files/Boost/boost_1_53_0/stage/lib', 'C:/Python27/libs' ] )

   target = 'Hello.pyd'
else:
   print platform.system() + " not supported"

e.Program(target = 'ActObj', source = 'Main.cpp')
