import platform, shutil

e = Environment()

debug = ARGUMENTS.get('debug', 0)

if(platform.system() == "Linux"):
   print "platform:linux"
   e.Append( CPPFLAGS = '-ansi -pthread -Wall -pedantic -Wno-long-long -std=c++0x' )
   e.Append( LINKFLAGS = '-Wall -pthread' )
   e.Append( CPPPATH = ['/usr/local/include'] )
   e.Append( LIBPATH = ['/usr/local/lib'] )
   e.Append( LIBS = [ 'boost_thread','boost_signals','boost_unit_test_framework' ] )
   e.Append( CCFLAGS = ['-D _LINPLATFORM'] )
   if int(debug):
	e.Append(CCFLAGS = '-D _DEBUG')

elif(platform.system() == "Windows"):
   e.Append( CPPFLAGS = ' /EHsc /D "WIN32" /D "_WIN32_WINNT#0x501" /D "_CONSOLE" /W4 /MD' )
   e.Append( CPPPATH = [ 'c:/Program Files/Boost/boost_1_53_0'] )
   e.Append( LIBPATH = [ 'C:/Program Files/Boost/boost_1_53_0/stage/lib' ] )
   e.Append( CCFLAGS = ['-D_WINPLATFORM'] )
   if int(debug):
	e.Append(CCFLAGS = '-D_DEBUG')
else:
   print platform.system() + " not supported"

if int(debug):
	e.Program(target = 'ActObjDeb', source = 'Tests.cpp')
else: 
	e.Program(target = 'ActObj', source = 'Tests.cpp')
