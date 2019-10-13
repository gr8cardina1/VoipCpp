#ifndef __CONDVAR_HPP
#define __CONDVAR_HPP
#pragma interface
class PCondVar : public PMutex {
	PCLASSINFO ( PCondVar, PMutex );
public:
	PCondVar ( PMutex & );
	virtual void Wait ( );
	virtual BOOL Wait ( const PTimeInterval & timeout );
	virtual void Signal ( );
	virtual BOOL WillBlock ( ) const;
private:
	PMutex & m;
	PCondVar ( const PCondVar & );
	PCondVar & operator= ( const PCondVar & );
};
#endif
