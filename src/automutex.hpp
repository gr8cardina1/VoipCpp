#pragma interface
#ifndef __AUTOMUTEX_HPP
#define __AUTOMUTEX_HPP
class AutoMutex {
	PMutex * m;
	AutoMutex ( const AutoMutex & );
	AutoMutex & operator= ( const AutoMutex & );
public:
	AutoMutex ( PMutex * mm );
	AutoMutex ( PMutex & mm );
	~AutoMutex ( );
};
class AntiMutex {
	PMutex * m;
	AntiMutex ( const AntiMutex & );
	AntiMutex & operator= ( const AntiMutex & );
public:
	AntiMutex ( PMutex * mm );
	AntiMutex ( PMutex & mm );
	~AntiMutex ( );
};
#endif
