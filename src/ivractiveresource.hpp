#ifndef IVR_ACTIVERESOURCE_HPP_
#define IVR_ACTIVERESOURCE_HPP_
#pragma interface


class IVRLanguageResource : public PHTTPResource, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( IVRLanguageResource, PHTTPResource )
public:
	IVRLanguageResource ( );
protected:
	PString LoadText ( PHTTPRequest & );
	BOOL LoadHeaders ( PHTTPRequest & request );

    virtual BOOL OnGETOrHEAD(
      PHTTPServer & server,       // HTTP server that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info,     // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo,
      BOOL  IsGet
    );

	void selectLanguage();

private:
	ss :: string m_language;
};


class IVRActiveResource : public PHTTPResource, public Allocatable < __SS_ALLOCATOR > {
	PCLASSINFO ( IVRActiveResource, PHTTPResource )
public:
	IVRActiveResource ( );
protected:
	PString LoadText ( PHTTPRequest & );
	BOOL LoadHeaders ( PHTTPRequest & request );

    virtual BOOL OnGETOrHEAD(
      PHTTPServer & server,       // HTTP server that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info,     // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo,
      BOOL  IsGet
    );

private:
	ss :: string m_key;
	ss :: string m_account;
	ss :: string m_password;
};

class IVRExpectedTimeResource : public PHTTPResource, public Allocatable < __SS_ALLOCATOR > {
    PCLASSINFO ( IVRActiveResource, PHTTPResource )
public:
    IVRExpectedTimeResource ();
protected:

    BOOL LoadHeaders ( PHTTPRequest & request );
    PString LoadText ( PHTTPRequest & );

    virtual BOOL OnGETOrHEAD(
      PHTTPServer & server,       // HTTP server that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info,     // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo,
      BOOL  IsGet
    );
private:

    ss :: string m_account;
	ss :: string m_password;
    ss :: string m_transferNumber;
};

class CHTTPServer : public PHTTPServer, public Allocatable < __SS_ALLOCATOR >
{
	PCLASSINFO ( CHTTPServer, PHTTPServer )
public:
	CHTTPServer();

    CHTTPServer(
     const PHTTPSpace & urlSpace  // Name space to use for URLs received.
    );
    virtual BOOL ProcessCommand();
    virtual BOOL OnGET(
      const PURL & url,                    // Universal Resource Locator for document.
      const PMIMEInfo & info,              // Extra MIME information in command.
      const PHTTPConnectionInfo & conInfo
    );
    virtual PString ReadEntityBody();

};

class CHTTPConnectionInfo : public PHTTPConnectionInfo
{

	friend class CHTTPServer;
};

class DigitsToString
{
public:
	DigitsToString( double digits );

//	StringVector getFileNameArray() const;

	int getBillions() const;
	int getMillions() const;
	int getThousands() const;
	int getHundreds() const;
	int getUnits() const;
	int getCents() const;

protected:

	void DivideByLexems();


protected:

	double m_digits;

	int m_billionsCount;
	int m_millionsCount;
	int m_thousandsCount;
	int m_hundredsCount;
	int m_dollarsCount;
	int m_centsCount;
//	StringVector m_lexems;
};

class CreateVXML : public Allocatable < __SS_ALLOCATOR >
{
public:
	CreateVXML( int retCode, const ss :: string& lang );
	virtual ~CreateVXML() {};
	virtual ss :: string getVXml();

protected:
    virtual void genErrorCode();
	virtual void genVXml() = 0;
    ss :: string getEnDigit( int iDigit ) const ;
    void getEnFirstHandred( PXMLElement * elem, PXMLElement * elemForm, int iDigit ) const ;

	void addPromptAndAudioData( PXMLElement * elem, PXMLElement * elemForm, const char* data ) const;
	void addGotoTag( PXMLElement * elem, PXMLElement * elemForm, const char* strAccount) const;
	void addVarTag( PXMLElement * elem, PXMLElement * elemForm,
                    const ss :: string& varName, const ss :: string& varValue/*,
                    const ss :: string& varPin, const ss :: string& varPinValue*/ ) const;

protected:
	PXML xml;
	ss :: string m_lang;
	int m_ret;
};

class CreateAuthVXML : public CreateVXML
{
public:
	CreateAuthVXML( double & strCost,
                    const ss :: string& account, const ss :: string& password,
                    int ret, const ss :: string& lang = "en" );
	virtual ~CreateAuthVXML() {};

protected:
	virtual void genVXml();

//	void genVXml( const ss :: string& lang );


	void genEnXml();
	void genRuXml();

	ss :: string getRuDigitPol( int iDigit, const ss :: string& nextWord ) const ;
	ss :: string getRuLastDigit( int iDigit ) const;
	ss :: string getRuFileUnits( int iDigit, const ss :: string& unitsName ) const;
	ss :: string getRuFileDigits( int iDigits, const ss :: string& unitsName ) const;

    void genMenu( PXMLElement * elemRoot );
    void genFormTransferRinging( PXMLElement * elemRoot );
    void genFormToDeposit( PXMLElement * elemRoot );
    void genFormExit( PXMLElement * elemRoot );

	static bool m_isFoolMap;
	static std :: set < ss :: string > m_setRuWords;
	static void fillRuMap();
	static bool isWords(const ss :: string & nextWord);
protected:
	double m_digits;
	ss :: string m_account;
    ss :: string m_password;
};

class CreateSelectLangVXML : public CreateVXML
{
public:
	CreateSelectLangVXML( int retCode, const ss :: string& lang = "en" );
	virtual ~CreateSelectLangVXML() {};
//	virtual ss :: string getVXml();

protected:
	virtual void genVXml();
};

class CreateExpectedTimeVXML : public CreateVXML
{
public:
    CreateExpectedTimeVXML ( double times, int retCode, const ss :: string& lang = "en" );
    virtual ~CreateExpectedTimeVXML () {};

protected:
    virtual void genVXml();

private:
    double m_times;

};







#endif /*IVR_ACTIVERESOURCE_HPP_*/
