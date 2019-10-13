#ifndef __NEW_H
#define __NEW_H
class opengate : public PServiceProcess {
	PCLASSINFO ( opengate, PServiceProcess )
	public:
	opengate ( );
	~opengate ( );
	virtual BOOL OnStart ( );
	virtual void OnControl ( );
	void Main ( );
};
#endif
