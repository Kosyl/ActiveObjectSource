import platform, shutil

e = Environment()

if(platform.system() == "Linux"):
   e.Append( CPPFLAGS = '-Wall -pedantic -pthread -Wno-long-long' )
   e.Append( LINKFLAGS = '-Wall -pthread' )
   e.Append( CPPPATH = ['/usr/include/python2.7'] )
   e.Append( LIBPATH = ['/usr/lib/python2.7'] )
   e.Append( LIBS = [ 'boost_python' ] )

elif(platform.system() == "Windows"):
   e.Append( CPPFLAGS = ' /EHsc /D "WIN32" /D "_WIN32_WINNT#0x501" /D "_CONSOLE" /W4 /MD' )
   e.Append( CPPPATH = [ 'c:/Program Files/Boost/boost_1_53_0'] )
   e.Append( LIBPATH = [ 'C:/Program Files/Boost/boost_1_53_0/stage/lib' ] )

else:
   print platform.system() + " not supported"

debug = ARGUMENTS.get('debug', 0)
if int(debug):
	e.Append(CCFLAGS = '-D_DEBUG')
	e.Program(target = 'ActObjDeb', source = 'Main.cpp')
else: 
	e.Program(target = 'ActObj', source = 'Main.cpp')
