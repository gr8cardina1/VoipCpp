#pragma implementation
#include "ss.hpp"
#include "allocatable.hpp"
#include <ptlib.h>
#include <ptclib/http.h>
#include <ptlib/sockets.h>
#include "aftertask.hpp"
#include "ipport.hpp"
#include "icqcontact.hpp"
#include "radgwinfo.hpp"
#include "Conf.hpp"
#include <ptlib/svcproc.h>
#include <ptclib/pxml.h>

#include "tarifround.hpp"
#include "tarifinfo.hpp"
#include "pricedata.hpp"
#include "requestinfo.hpp"

#include "ivractiveresource.hpp"
#include <cstring>

bool CreateAuthVXML :: m_isFoolMap = false;
std :: set < ss :: string > CreateAuthVXML :: m_setRuWords;


//==================================================================================
IVRLanguageResource :: IVRLanguageResource ( ) :
	PHTTPResource ( "lang.vxml", "text/xml" ) { }

PString IVRLanguageResource :: LoadText ( PHTTPRequest & )
{
	int retCode = -3;

	if( m_language == "ru" || m_language == "en")
	{
		retCode = 0;
	}

	CreateSelectLangVXML createVXml( retCode, m_language );
	ss :: string retXml = createVXml.getVXml();

	PSYSTEMLOG(Info, "IVRLanguageResource: lang: " << m_language << "; response: " << retXml );
	return retXml.c_str();
}

BOOL IVRLanguageResource :: LoadHeaders ( PHTTPRequest & request )
{
    ss :: string val = static_cast < const char * > ( request.url.AsString( PURL :: URIOnly ) );
	ss :: string :: size_type start_val = val.find('?');
	if ( start_val != ss :: string :: npos )
	{
		ss :: string val1 = val.substr( start_val );
		PSYSTEMLOG( Info, "We are find '?'." << val1 );
		ss :: string :: size_type start_key = val1.find('=');
		if ( start_key != ss :: string :: npos )
		{
			ss :: string key = val1.substr( 1, start_key-1 );
			ss :: string langData = val1.substr( start_key + 1 );

			PSYSTEMLOG( Info, "IVRLanguageResource :: LoadHeaders: value = " << langData );
			if ( langData == "1" )
			{
				m_language = "en";
			}
			else if( langData == "2" )
			{
				m_language = "ru";
			}
			PSYSTEMLOG( Info, "IVRLanguageResource :: LoadHeaders: lang = " << m_language );
		}
		else
			PSYSTEMLOG(Error, "IVRLanguageResource :: Do not find '='.");
	}
	else
		PSYSTEMLOG(Error, "IVRLanguageResource :: Do not find '?'.");

	return true;
}

BOOL IVRLanguageResource :: OnGETOrHEAD( PHTTPServer & server, const PURL & url, const PMIMEInfo & info,
      const PHTTPConnectionInfo & connectInfo, BOOL isGet )
{
  // Nede to split songle if into 2 so the Tornado compiler won't end with
  // 'internal compiler error'

  PSYSTEMLOG(Info, "IVRLanguageResource :: OnGETOrHEAD");

  if (isGet && info.Contains(PHTTP::IfModifiedSinceTag))
    if (!IsModifiedSince(PTime(info[PHTTP::IfModifiedSinceTag])))
      return server.OnError(PHTTP::NotModified, url.AsString(), connectInfo);

  PHTTPRequest * request = CreateRequest(url,
                                         info,
                                         connectInfo.GetMultipartFormInfo(),
                                         server);

  BOOL retVal = TRUE;
//  if (CheckAuthority(server, *request, connectInfo))
{
    retVal = FALSE;
    server.SetDefaultMIMEInfo(request->outMIME, connectInfo);

    PTime expiryDate;
    if (GetExpirationDate(expiryDate))
      request->outMIME.SetAt(PHTTP::ExpiresTag,
                              expiryDate.AsString(PTime::RFC1123, PTime::GMT));

    if (!LoadHeaders(*request))
      retVal = server.OnError(request->code, url.AsString(), connectInfo);
    else if (!isGet)
      retVal = request->outMIME.Contains(PHTTP::ContentLengthTag);
    else {
      hitCount++;
      retVal = OnGETData(server, url, connectInfo, *request);
    }
  }

  delete request;
  return retVal;
}


//==================================================================================

IVRActiveResource :: IVRActiveResource ( ) :
	PHTTPResource ( "authenticate.vxml", "text/xml" ) { 	}

PString IVRActiveResource :: LoadText ( PHTTPRequest & request) {
	double retString;
	ss :: string lang;
    PSYSTEMLOG(Info, "IVRActiveResource :: LoadText: acc = " << m_account << "; passw = " << m_password);
	int iRet = conf -> loadIVRData ( m_account, m_password, retString, lang );
    PSYSTEMLOG(Info, "IVRActiveResource :: LoadText: ret = " << iRet << "; retStr = " << retString);

	CreateAuthVXML createVXml( retString, m_account, m_password, iRet, lang );
	ss :: string retXml = createVXml.getVXml();

	PSYSTEMLOG(Info, "IVRActiveResource: request: " << request.entityBody << "; response: " << retXml );
	return retXml.c_str();
}

BOOL IVRActiveResource :: LoadHeaders ( PHTTPRequest & request ) {
    ss :: string val = static_cast < const char * > ( request.url.AsString( PURL :: URIOnly ) );

    PSYSTEMLOG(Info, "IVRActiveResource :: LoadHeaders: " << val.c_str() );

	ss :: string :: size_type start_val = val.find('?');
    if ( start_val != ss :: string :: npos )
	{
		ss :: string val1 = val.substr( start_val );
		PSYSTEMLOG( Info, "We are find '?'." << val1 );
		ss :: string :: size_type start_key = val1.find('=');
		if ( start_key != ss :: string :: npos )
		{
			m_key = val1.substr( 1, start_key-1 );
			ss :: string value = val1.substr( start_key + 1 );
			ss :: string :: size_type endOfacc = value.find("%23");
			if ( endOfacc != ss :: string :: npos )
			{
				m_account = value.substr( 0, endOfacc );
				PSYSTEMLOG( Info, "IVRActiveResource :: LoadHeaders: value = " << value );
				value = value.substr( endOfacc + ( sizeof("%23") - 1 ) );
				ss :: string :: size_type endOfpass = value.find("%23");
				PSYSTEMLOG( Info, "IVRActiveResource :: LoadHeaders: value = " << value );
				if ( endOfpass != ss :: string :: npos )
				{
					m_password = value.substr( 0, endOfpass );
					PSYSTEMLOG( Info, "IVRActiveResource :: LoadHeaders: key = " << m_key << "; account = " << m_account << ", password = " << m_password);
				}
			}
		}
		else
			PSYSTEMLOG(Error, "Do not find '='.");
	}
	else
		PSYSTEMLOG(Error, "Do not find '?'.");

	return true;
}

BOOL IVRActiveResource::OnGETOrHEAD(PHTTPServer & server,
                           const PURL & url,
                      const PMIMEInfo & info,
            const PHTTPConnectionInfo & connectInfo,
                                   BOOL isGET)
{
  // Nede to split songle if into 2 so the Tornado compiler won't end with
  // 'internal compiler error'
    PSYSTEMLOG(Info, "IVRActiveResource :: OnGETOrHEAD");
  if (isGET && info.Contains(PHTTP::IfModifiedSinceTag))
    if (!IsModifiedSince(PTime(info[PHTTP::IfModifiedSinceTag])))
      return server.OnError(PHTTP::NotModified, url.AsString(), connectInfo);

  PHTTPRequest * request = CreateRequest(url,
                                         info,
                                         connectInfo.GetMultipartFormInfo(),
                                         server);

  BOOL retVal = TRUE;
//  if (CheckAuthority(server, *request, connectInfo))
{
    retVal = FALSE;
    server.SetDefaultMIMEInfo(request->outMIME, connectInfo);

    PTime expiryDate;
    if (GetExpirationDate(expiryDate))
      request->outMIME.SetAt(PHTTP::ExpiresTag,
                              expiryDate.AsString(PTime::RFC1123, PTime::GMT));

    if (!LoadHeaders(*request))
      retVal = server.OnError(request->code, url.AsString(), connectInfo);
    else if (!isGET)
      retVal = request->outMIME.Contains(PHTTP::ContentLengthTag);
    else {
      hitCount++;
      retVal = OnGETData(server, url, connectInfo, *request);
    }
  }

  delete request;
  return retVal;
}

//----------------------------------------------------------------------------------------------------------------------

IVRExpectedTimeResource :: IVRExpectedTimeResource () :
	PHTTPResource ( "expected_time.vxml", "text/xml" )
{
    PSYSTEMLOG(Info, "IVRExpectedTimeResource :: IVRExpectedTimeResource");
}

BOOL IVRExpectedTimeResource :: LoadHeaders ( PHTTPRequest & request )
{
    ss :: string testString = static_cast < const char * > ( request.url.AsString( PURL :: URIOnly ) );

    PSYSTEMLOG(Info, "IVRExpectedTimeResource :: LoadHeaders: " << testString.c_str() );
/*
    ss :: string :: size_type start_val = val.find('?');
    if ( start_val != ss :: string :: npos )
    {
        ss :: string val1 = val.substr( start_val );
        PSYSTEMLOG( Info, "We are find '?'." << val1 );
        ss :: string :: size_type start_key = val1.find('=');
        if ( start_key != ss :: string :: npos )
        {
            ss :: string value = val1.substr( start_key + 1 );
            ss :: string :: size_type endOfacc = value.find('&');
            if ( endOfacc != ss :: string :: npos )
            {
                m_account = value.substr( 0, endOfacc );
                PSYSTEMLOG( Info, "IVRActiveResource :: LoadHeaders: value = " << value );
                value = value.substr( endOfacc + 1 );
                ss :: string :: size_type endOfpass = value.find('&');
                PSYSTEMLOG( Info, "IVRActiveResource :: LoadHeaders: value = " << value );
                if ( endOfpass != ss :: string :: npos )
                {
                    m_password = value.substr( 0, endOfpass );
                    PSYSTEMLOG( Info, "IVRActiveResource :: LoadHeaders: account = " << m_account << ", password = " << m_password);
                }
            }
        }
        else
            PSYSTEMLOG(Error, "Do not find '='.");
    }
    else
        PSYSTEMLOG(Error, "Do not find '?'.");
*/
    ss :: string :: size_type start_val = testString.find('?');

    if ( start_val != ss :: string :: npos )
    {
        ss :: string val1 = testString.substr( start_val );
        ss :: string :: size_type start_key = val1.find('=');
        if ( start_key != string :: npos )
        {
            val1 = val1.substr( start_key + 1 );
            ss :: string :: size_type start_key1 = val1.find('&');
            if ( start_key1 != string :: npos )
            {
                m_account = val1.substr( 0, start_key1 );
                ss :: string :: size_type start_key2 = val1.find('=');
                if ( start_key2 != string :: npos )
                {
                    val1 = val1.substr( start_key2 + 1 );
                    ss :: string :: size_type start_key3 = val1.find('&');
                    if (start_key3 != string :: npos)
                    {
                        m_password = val1.substr( 0, start_key3 );
                        ss :: string :: size_type start_key4 = val1.find('='); // TransferNumber
                        if ( start_key4 != ss :: string :: npos  )
                        {
                            val1 = val1.substr( start_key4 + 1 );
                            ss :: string :: size_type start_key5 = val1.find('&');
                            if (start_key5 != string :: npos)
                            {
                                m_transferNumber = val1.substr( 0, start_key5 );
                            }
                        }

                    }
                }
            }
        }
        PSYSTEMLOG( Info, "test:" << testString << "; useracc = " << m_account << "; userpin = " << m_password );
    }

    return true;
}

PString IVRExpectedTimeResource :: LoadText ( PHTTPRequest &  request)
{
    double retString;
    ss :: string lang;
    PSYSTEMLOG(Info, "IVRExpectedTimeResource :: LoadText: acc = " << m_account << "; passw = " << m_password);
    RequestInfo ri;

    /*int iRet1 = */conf -> loadIVRData ( m_account, m_password, retString, lang );
    ri .acctn = m_account;
    ri.pass = m_password;
    ri.calledStationId = m_transferNumber;


    int iRet2 = conf -> getCreditTimeIVR ( ri );
        /*char strCost[255];
        sprintf( strCost, "./sound/%s_card_expired.au.wav", m_lang.c_str() );
        addPromptAndAudioData( elem, elemForm, strCost );
        */
//        ri.dnis;
//        ri.ani;

    PSYSTEMLOG(Info, "IVRExpectedTimeResource :: LoadText: ret = " << iRet2 << "; creditTime = " << ri.creditTime );

    CreateExpectedTimeVXML createVXml( ri.creditTime, iRet2, lang );
    ss :: string retXml = createVXml.getVXml();

    PSYSTEMLOG(Info, "IVRExpectedTimeResource: request: " << request.entityBody << "; response: " << retXml );
    return retXml.c_str();
}

BOOL IVRExpectedTimeResource :: OnGETOrHEAD(
      PHTTPServer & server,       // HTTP server that received the request
      const PURL & url,           // Universal Resource Locator for document.
      const PMIMEInfo & info,     // Extra MIME information in command.
      const PHTTPConnectionInfo & connectInfo,
      BOOL  isGET
    )
{
    PSYSTEMLOG(Info, "IVRExpectedTimeResource :: OnGETOrHEAD");
  if (isGET && info.Contains(PHTTP::IfModifiedSinceTag))
    if (!IsModifiedSince(PTime(info[PHTTP::IfModifiedSinceTag])))
      return server.OnError(PHTTP::NotModified, url.AsString(), connectInfo);

  PHTTPRequest * request = CreateRequest(url,
                                         info,
                                         connectInfo.GetMultipartFormInfo(),
                                         server);

  BOOL retVal = TRUE;
//  if (CheckAuthority(server, *request, connectInfo))
  {
    retVal = FALSE;
    server.SetDefaultMIMEInfo(request->outMIME, connectInfo);

    PTime expiryDate;
    if (GetExpirationDate(expiryDate))
      request->outMIME.SetAt(PHTTP::ExpiresTag,
                              expiryDate.AsString(PTime::RFC1123, PTime::GMT));

    if (!LoadHeaders(*request))
      retVal = server.OnError(request->code, url.AsString(), connectInfo);
    else if (!isGET)
      retVal = request->outMIME.Contains(PHTTP::ContentLengthTag);
    else {
      hitCount++;
      retVal = OnGETData(server, url, connectInfo, *request);
    }
  }

  delete request;
  return retVal;
}

//----------------------------------------------------------------------------------------------------------------------

CHTTPServer :: CHTTPServer()
{
}

CHTTPServer :: CHTTPServer( const PHTTPSpace & urlSpace ) : PHTTPServer( urlSpace )
{

}

BOOL CHTTPServer :: ProcessCommand()
{
	PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand()");
  PString args;
  PINDEX cmd;

	CHTTPConnectionInfo& connectInfo1 = *(CHTTPConnectionInfo*)&connectInfo;
	// Dlya togo, chtoby videt protected members of PHTTPConnectionInfo

  // if this is not the first command received by this socket, then set
  // the read timeout appropriately.
  if (transactionCount > 0)
    SetReadTimeout(nextTimeout);

  // this will only return false upon timeout or completely invalid command
  	if (!ReadCommand(cmd, args))
  	{
		PSYSTEMLOG(Info, "ReadCommand is returned false");
  		return FALSE;
  	}

  connectInfo1.commandCode = (Commands)cmd;
  if (cmd < NumCommands)
    connectInfo1.commandName = commandNames[cmd];
  else {
    PINDEX spacePos = args.Find(' ');
    connectInfo1.commandName = args.Left(spacePos);
    args = args.Mid(spacePos);
  }

	PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). " << connectInfo1.GetCommandName() );
	PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). " << connectInfo.GetCommandName() );


  // if no tokens, error
  if (args.IsEmpty()) {
    OnError(BadRequest, args, connectInfo1);
	PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). Bad request" );
    return FALSE;
  }

	if (!connectInfo1.Initialise(*this, args))
	{
		PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). Errors of init connectInfo " );
		return FALSE;
	}

  // now that we've decided we did receive a HTTP request, increment the
  // count of transactions
  transactionCount++;
  nextTimeout = connectInfo1.GetPersistenceTimeout();

  PIPSocket * socket = GetSocket();
  WORD myPort = (WORD)(socket != NULL ? socket->GetPort() : 80);

  // the URL that comes with Connect requests is not quite kosher, so
  // mangle it into a proper URL and do NOT close the connection.
  // for all other commands, close the read connection if not persistant
  if (cmd == CONNECT)
    connectInfo1.url = "https://" + args;
  else {
    connectInfo1.url = args;
    if (connectInfo1.url.GetPort() == 0)
      connectInfo1.url.SetPort(myPort);
  }
	PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). url1 = '" << connectInfo1.GetURL() << '\'' );
//	PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). url2 = '" << args << "'; port = " << myPort );

  BOOL persist;

  // make sure the form info is reset for each new operation
  connectInfo1.ResetMultipartFormInfo();

  // If the incoming URL is of a proxy type then call OnProxy() which will
  // probably just go OnError(). Even if a full URL is provided in the
  // command we should check to see if it is a local server request and process
  // it anyway even though we are not a proxy. The usage of GetHostName()
  // below are to catch every way of specifying the host (name, alias, any of
  // several IP numbers etc).
  const PURL & url = connectInfo1.GetURL();
  if (url.GetScheme() != "http" ||
      (url.GetPort() != 0 && url.GetPort() != myPort) ||
      (!url.GetHostName() && !PIPSocket::IsLocalHost(url.GetHostName())))
    persist = OnProxy(connectInfo1);
  else {
    connectInfo1.entityBody = ReadEntityBody();
//	PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). body = " << connectInfo1.entityBody );

    // Handle the local request
    PStringToString postData;
    switch (cmd) {
      case GET :
		PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). get" );
        persist = OnGET(url, connectInfo1.GetMIME(), connectInfo1);
        break;

      case HEAD :
		PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). head" );
        persist = OnHEAD(url, connectInfo1.GetMIME(), connectInfo1);
        break;

      case POST :
        {
			PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). post." );
          // check for multi-part form POSTs
          PString postType = (connectInfo1.GetMIME())(ContentTypeTag);
          if (postType.Find("multipart/form-data") == 0)
            connectInfo1.DecodeMultipartFormInfo(postType, connectInfo1.entityBody);
          else  // if (postType *= "x-www-form-urlencoded)
            PURL::SplitQueryVars(connectInfo1.entityBody, postData);
        }
        persist = OnPOST(url, connectInfo1.GetMIME(), postData, connectInfo1);
        break;

      case P_MAX_INDEX:
      default:
        persist = OnUnknown(args, connectInfo1);
    }
  }

  flush();

  // if the function just indicated that the connection is to persist,
  // and so did the client, then return TRUE. Note that all of the OnXXXX
  // routines above must make sure that their return value is FALSE if
  // if there was no ContentLength field in the response. This ensures that
  // we always close the socket so the client will get the correct end of file
  if (persist && connectInfo1.IsPersistant()) {
    unsigned max = connectInfo1.GetPersistenceMaximumTransations();
    if (max == 0 || transactionCount < max)
    {
		PSYSTEMLOG(Info, "CHTTPServer :: ProcessCommand(). good. " );
    	return TRUE;
    }
  }

  PSYSTEMLOG(Error, "HTTPServer\tConnection end: " << connectInfo1.IsPersistant());

  // close the output stream now and return FALSE
  Shutdown(ShutdownWrite);
  return FALSE;
}

PString CHTTPServer::ReadEntityBody()
{
  	if (connectInfo.GetMajorVersion() < 1)
  	{
	  	PSYSTEMLOG(Error, "CHTTPServer::ReadEntityBody: version = " << connectInfo.GetMajorVersion());
  		return PString();
  	}

  PString entityBody;
  long contentLength = connectInfo.GetEntityBodyLength();
  // a content length of > 0 means read explicit length
  // a content length of < 0 means read until EOF
  // a content length of 0 means read nothing
	  	PSYSTEMLOG(Info, "CHTTPServer::ReadEntityBody: contentLength = " << contentLength);
  int count = 0;
  if (contentLength > 0) {
    entityBody = ReadString((PINDEX)contentLength);
	  	PSYSTEMLOG(Info, "CHTTPServer::ReadEntityBody:1 eb = " << entityBody);
  } else if (contentLength == -2) {
    ReadLine(entityBody, FALSE);
	  	PSYSTEMLOG(Info, "CHTTPServer::ReadEntityBody:2 eb = " << entityBody);
  } else if (contentLength < 0) {
    while (Read(entityBody.GetPointer(count+1000)+count, 1000))
      count += GetLastReadCount();
    entityBody.SetSize(count+1);
	  	PSYSTEMLOG(Info, "CHTTPServer::ReadEntityBody:1 eb = " << entityBody << "; count = " << count + 1);
  }

  // close the connection, if not persistant
  if (!connectInfo.IsPersistant()) {
	  	PSYSTEMLOG(Info, "CHTTPServer::ReadEntityBody: not persistent. close connection.");
    PIPSocket * socket = GetSocket();
    if (socket != NULL)
      socket->Shutdown(PIPSocket::ShutdownRead);
  }

  return entityBody;
}

BOOL CHTTPServer::OnGET(const PURL & url,
                   const PMIMEInfo & info,
         const PHTTPConnectionInfo & connectInfo)
{
  urlSpace.StartRead();
  PHTTPResource * resource = urlSpace.FindResource(url);
  if (resource == NULL) {
	  	PSYSTEMLOG(Info, "CHTTPServer::OnGET: not persistent. close connection.");
    urlSpace.EndRead();
    return OnError(NotFound, url.AsString(), connectInfo);
  }

  BOOL retval = resource->OnGET(*this, url, info, connectInfo);
  urlSpace.EndRead();
	  	PSYSTEMLOG(Info, "CHTTPServer::OnGET: end. retval = " << retval);
  return retval;
}


///====================================================================================
DigitsToString :: DigitsToString( double digits ) :
	m_digits( digits ),
	m_billionsCount(0),
	m_millionsCount(0),
	m_thousandsCount(0),
	m_hundredsCount(0),
	m_dollarsCount(0),
	m_centsCount(0)

{
	DivideByLexems();
}
///====================================================================================
//StringVector DigitsToString :: getFileNameArray() const
//{
//	return m_lexems;
//}
///====================================================================================
void DigitsToString :: DivideByLexems()
{
//	char * pEnd;
	//double dbCost = strtod(m_digits.c_str(), &pEnd);

	m_dollarsCount 	= int( m_digits );
	int priceInCents	= int ( m_digits * 100 );
	m_centsCount 		=  priceInCents - m_dollarsCount * 100;

	if ( m_dollarsCount >= 1000000000 )
	{ // billion
		m_billionsCount = m_dollarsCount / 1000000000;
		m_dollarsCount -= m_billionsCount * 1000000000;
	}

 	if ( m_dollarsCount >= 1000000 )
	{ // million
		m_millionsCount = m_dollarsCount / 1000000;
		m_dollarsCount -= m_millionsCount * 1000000;
	}

 	if ( m_dollarsCount >= 1000 )
	{ // thousand
		m_thousandsCount = m_dollarsCount / 1000;
		m_dollarsCount -= m_thousandsCount * 1000;
	}

 	if ( m_dollarsCount >= 100 )
	{ // hundred
		m_hundredsCount = m_dollarsCount / 100;
		m_dollarsCount -= m_hundredsCount * 100;
	}

	PSYSTEMLOG( Info, "cost = " << m_digits << "; billion = " << m_billionsCount << "; million = " << m_millionsCount <<
		"; thousand = " << m_thousandsCount << "; hundred = " << m_hundredsCount << "; units = " << m_dollarsCount << "; cents = " << m_centsCount );
}
///====================================================================================
int DigitsToString :: getBillions() const
{
	return m_billionsCount;
}
int DigitsToString :: getMillions() const
{
	return m_millionsCount;
}

int DigitsToString :: getThousands() const
{
	return m_thousandsCount;
}

int DigitsToString :: getHundreds() const
{
	return m_hundredsCount;
}

int DigitsToString :: getUnits() const
{
	return m_dollarsCount;
}

int DigitsToString :: getCents() const
{
	return m_centsCount;
}
///====================================================================================

CreateVXML :: CreateVXML ( int retCode, const ss :: string& lang /*= "en"*/ ) :
	xml ( PXML :: Indent | PXML :: NewLineAfterElement ),
	m_lang ( lang ),
	m_ret( retCode )
{

}
///====================================================================================
ss :: string CreateVXML :: getVXml()
{
	ss :: string retXml;
	PSYSTEMLOG(Info, "CreateVXML :: getVXml");

	if ( m_ret < 0 )
	{
		genErrorCode();
	}
	else
	{
		genVXml();
	}

	ss :: ostringstream os;
	xml.PrintOn ( os );
	return os.str ( ).c_str ( );
}
///====================================================================================
void CreateVXML :: genErrorCode()
{
	PSYSTEMLOG(Info, "CreateVXML :: genErrorCode: " << m_ret);
	PXMLElement * elem = new PXMLElement ( 0, "vxml" );
	elem -> SetAttribute ( "version", "1.0" );
	xml.SetRootElement ( elem );
	PXMLElement * elemForm = new PXMLElement ( elem, "form" );
	elemForm -> SetAttribute ( "id", "balance" );
	elem -> AddChild ( elemForm );

	switch ( m_ret )
	{
		case -1:
		{
			PXMLElement * elemPrompt = new PXMLElement ( elem, "prompt" );
			elemForm -> AddChild ( elemPrompt );
			PXMLElement * elemAudio = new PXMLElement ( elem, "audio" );
			elemAudio -> SetAttribute ( "src", "./sound/auth_fail_retry_number.au.wav" );
			elemPrompt -> AddChild ( elemAudio );
			break;
		}
		case -2:
		{
			PXMLElement * elemPrompt = new PXMLElement ( elem, "prompt" );
			elemForm -> AddChild ( elemPrompt );
			PXMLElement * elemAudio = new PXMLElement ( elem, "audio" );
			elemAudio -> SetAttribute ( "src", "./sound/en_auth_fail_pin.au.wav" );
			elemPrompt -> AddChild ( elemAudio );
			break;
		}
		case -3:
		{
			PXMLElement * elemPrompt = new PXMLElement ( elem, "prompt" );
			elemForm -> AddChild ( elemPrompt );
			PXMLElement * elemAudio = new PXMLElement ( elem, "audio" );
			elemAudio -> SetAttribute ( "src", "./sound/en_wrong_lang_sel.au.wav" );
			elemPrompt -> AddChild ( elemAudio );
			break;
		}
	}
	{
		char strAccount[255];

//		sprintf( strAccount, "./authenticated.vxml" );
		sprintf( strAccount, "authenticated" );
		addGotoTag(elem, elemForm, strAccount);
	}
}
///====================================================================================
void CreateVXML :: addPromptAndAudioData( PXMLElement * elem, PXMLElement * elemForm, const char* data ) const
{
	PXMLElement * elemPrompt = new PXMLElement ( elem, "prompt" );
	elemForm -> AddChild ( elemPrompt );
	PXMLElement * elemAudio = new PXMLElement ( elem, "audio" );
	elemAudio -> SetAttribute ( "src", data );
	elemPrompt -> AddChild ( elemAudio );
}
///------------------------------------------------------------------------------------
void CreateVXML :: addGotoTag( PXMLElement * elem, PXMLElement * elemForm, const char* strPath ) const
{
	PXMLElement * elemGoto = new PXMLElement ( elem, "goto" );

	elemGoto -> SetAttribute ( "next", strPath );
//	elemGoto -> SetAttribute ( "account", "1234" );

	elemForm -> AddChild ( elemGoto );
}
///------------------------------------------------------------------------------------
void CreateVXML :: addVarTag( PXMLElement * elem, PXMLElement * elemForm,
                              const ss :: string& varName, const ss :: string& varValue
                               ) const
{
	PXMLElement * elemGoto = new PXMLElement ( elem, "var" );

	elemGoto -> SetAttribute ( "name", varName.c_str() );
    elemGoto -> SetAttribute ( "expr", varValue.c_str() );

	elemForm -> AddChild ( elemGoto );
}
///====================================================================================
ss :: string CreateVXML :: getEnDigit( int iDigit ) const
{
	switch( iDigit )
	{
	case 0:
		return "zero";
	case 1:
		return "one";
	case 2:
		return "two";
	case 3:
		return "three";
	case 4:
		return "four";
	case 5:
		return "five";
	case 6:
		return "six";
	case 7:
		return "seven";
	case 8:
		return "eight";
	case 9:
		return "nine";
	case 10:
		return "ten";
	case 11:
		return "eleven";
	case 12:
		return "twelve";
	case 13:
		return "thirteen";
	case 14:
		return "fourteen";
	case 15:
		return "fifteen";
	case 16:
		return "sixteen";
	case 17:
		return "seventeen";
	case 18:
		return "eighteen";
	case 19:
		return "nineteen";
	case 20:
		return "twenty";
	case 30:
		return "thirty";
	case 40:
		return "forty";
	case 50:
		return "fifty";
	case 60:
		return "sixty";
	case 70:
		return "seventy";
	case 80:
		return "eighty";
	case 90:
		return "ninety";
	}

	return "";
}

void CreateVXML :: getEnFirstHandred( PXMLElement * elem, PXMLElement * elemForm, int iDigit ) const
{
    char strCost[255];
    int tens = 0;
    if( iDigit > 19)
    {
        tens = (iDigit / 10) * 10;
        if( tens )
        {
            sprintf( strCost, "./sound/en_%s.au.wav", getEnDigit( tens ).c_str() );
            addPromptAndAudioData( elem, elemForm, strCost );
        }
    }
    int units = iDigit - tens;

    if(units)
    {
        sprintf( strCost, "./sound/en_%s.au.wav", getEnDigit( units ).c_str() );
        addPromptAndAudioData( elem, elemForm, strCost );
    }
}

void CreateAuthVXML :: fillRuMap()
{ // Etot set soderzhit ispolzuemye nami sushestvitelnye v muzhskom rode.
	if(m_isFoolMap)
	{
		return;
	}



	m_setRuWords.insert( "hour" );
	// minute --
	// second --
	// dollar -- (edinitsa!!!)
	// cent   --
	// thousand --

	m_setRuWords.insert( "cent" );
	m_isFoolMap = true;
}
bool CreateAuthVXML :: isWords(const ss :: string & nextWord)
{
	if ( false == m_isFoolMap )
		fillRuMap();

	return m_setRuWords.find( nextWord ) != m_setRuWords.end();
}

ss :: string CreateAuthVXML :: getRuDigitPol( int iDigit, const ss :: string & nextWord ) const
{
	bool isMans = isWords( nextWord );

	int lastDigit;
	if( iDigit < 10 )
		lastDigit = iDigit;
	else
		lastDigit = iDigit - ( iDigit / 10 ) * 10;

	switch ( lastDigit )
	{
	case 1:
	case 2:
		if (isMans)
			return "_m";
		else
			return "_f";
	}

	return "";
}

ss :: string CreateAuthVXML :: getRuLastDigit( int iDigit ) const
{
	int lastDigit;
	if( iDigit <= 20 && iDigit > 10)
		return "5-20";

	if( iDigit < 10 )
		lastDigit = iDigit;
	else
		lastDigit = iDigit - ( iDigit / 10 ) * 10;
	if ( lastDigit == 1 )
		return "";
	if ( lastDigit > 1 && lastDigit < 5 )
		return "2-4";

	return "5-20";
}

ss :: string CreateAuthVXML :: getRuFileUnits( int iDigit, const ss :: string& unitsName ) const
{
	char unitsRetName[255];
	ss :: string last = getRuLastDigit( iDigit );
	if( last.empty() )
		sprintf(unitsRetName, "./sound/ru_%s.au.wav", unitsName.c_str() );
	else
		sprintf(unitsRetName, "./sound/ru_%s_%ss.au.wav", last.c_str(), unitsName.c_str() );

	return unitsRetName;
}

ss :: string CreateAuthVXML :: getRuFileDigits( int iDigit, const ss :: string & nextWord ) const
{
	char digitsRetName[255];

	ss :: string suffix = getRuDigitPol( iDigit, nextWord );
	sprintf(digitsRetName, "./sound/ru_%d%s.au.wav", iDigit, suffix.c_str() );

	return digitsRetName;
}
///====================================================================================
CreateAuthVXML :: CreateAuthVXML( double & strCost,
                                  const ss :: string& account,
                                  const ss :: string& password,
                                  int ret,
                                  const ss :: string& lang ) :
	CreateVXML( ret, lang ),
	m_digits ( strCost ),
	m_account( account ),
    m_password(password)
{
    PSYSTEMLOG(Info, "CreateAuthVXML : strCost = " <<  strCost);
}

void  CreateAuthVXML :: genMenu( PXMLElement * elemRoot )
{
    if(!elemRoot)
        return;

    PXMLElement * elemForm = new PXMLElement ( elemRoot, "form" );
    elemForm -> SetAttribute ( "id", "balance" );
    elemRoot -> AddChild ( elemForm );

    PXMLElement * elemFilled = new PXMLElement ( elemRoot, "filled" );
    elemForm -> AddChild ( elemFilled );
    PXMLElement * elemChoice1 = new PXMLElement ( elemRoot, "choice" );
    elemChoice1 -> SetAttribute ( "dtmf", "1" );
    elemChoice1 -> SetAttribute ( "next", "#form1" );
    elemFilled -> AddChild ( elemChoice1 );

    PXMLElement * elemChoice2 = new PXMLElement ( elemRoot, "choice" );
    elemChoice2 -> SetAttribute ( "dtmf", "2" );
    elemChoice2 -> SetAttribute ( "next", "#form2" );
    elemFilled -> AddChild ( elemChoice2 );

    PXMLElement * elemNoMatch = new PXMLElement ( elemRoot, "nomatch" );
    elemForm -> AddChild ( elemNoMatch );

    PXMLElement * elemAudio = new PXMLElement ( elemRoot, "audio" );
    elemAudio -> SetAttribute ( "src", "./sound/en_you_have.au.wav" );
    elemNoMatch -> AddChild ( elemAudio );
    PXMLElement * elemGoto = new PXMLElement ( elemRoot, "goto" );
    elemGoto -> SetAttribute ( "next", "#form3" );
    elemNoMatch -> AddChild ( elemGoto );
}

void CreateAuthVXML :: genFormTransferRinging( PXMLElement * elemRoot )
{
    PXMLElement * elemForm = new PXMLElement ( elemRoot, "form1" );
    elemForm -> SetAttribute ( "id", "balance" );
    elemRoot -> AddChild ( elemForm );

    PXMLElement * elemBlock = new PXMLElement ( elemRoot, "block" );
    elemForm -> AddChild ( elemBlock );

    DigitsToString ds(m_digits);

    {
        char strCost[255];
        sprintf( strCost, "./sound/en_you_have.au.wav" );
        addPromptAndAudioData( elemRoot, elemBlock, strCost );
    }

    ss :: string strDigits;
    if ( ds.getThousands() )
    {
        char strCost[255];
        int iDigits = ds.getThousands();
        if( iDigits > 100 )
        {
            iDigits = iDigits / 100;
            strDigits = getEnDigit( iDigits );

            sprintf( strCost, "./sound/en_%s.au.wav", strDigits.c_str() );
            addPromptAndAudioData( elemRoot, elemBlock, strCost );
            addPromptAndAudioData( elemRoot, elemBlock, "./sound/en_hundred.au.wav" );

            iDigits = ds.getThousands() - iDigits * 100;
        }

        strDigits = getEnDigit( iDigits );
        sprintf( strCost, "./sound/en_%s.au.wav", strDigits.c_str() );
        addPromptAndAudioData( elemRoot, elemBlock, strCost );
        addPromptAndAudioData( elemRoot, elemBlock, "./sound/en_thousand.au.wav" );
    }
    if ( ds.getHundreds() )
    {
        char strCost[255];
        sprintf( strCost, "./sound/en_%s.au.wav", getEnDigit( ds.getHundreds() ).c_str() );
        addPromptAndAudioData( elemRoot, elemBlock, strCost );
        addPromptAndAudioData( elemRoot, elemBlock, "./sound/en_hundred.au.wav" );

    }
    if ( ds.getUnits() )
    {
        getEnFirstHandred(elemRoot, elemBlock, ds.getUnits());
    }

    if(int(m_digits) > 0 )
        addPromptAndAudioData( elemRoot, elemBlock, "./sound/en_dollars.au.wav" );
    if ( ds.getCents() )
    {
        getEnFirstHandred(elemRoot, elemForm, ds.getCents());
        addPromptAndAudioData( elemRoot, elemBlock, "./sound/en_cents.au.wav" );
    }
/* /// OLD. GOTO tag
    {
        char strAccount[255];

//		sprintf( strAccount, "./authenticated.vxml?account=%s", m_account.c_str() );
        sprintf( strAccount, "authenticated" );
//		sprintf( strAccount, "#authenticated" );
        addGotoTag(elemRoot, elemForm, strAccount);
    }
*/


/**
    ///
       <form id="form1">
                <block>
                        <prompt>
                            <audio src="./_one.wav"/>
                        </prompt>
                        <goto next="#root"/>
                </block>
        </form>
**/
    addGotoTag(elemRoot, elemForm, "#root");
}

void CreateAuthVXML :: genFormToDeposit( PXMLElement * elemRoot )
{
    PXMLElement * elemForm2 = new PXMLElement ( elemRoot, "form2" );
    //elemForm2 -> SetAttribute ( "id", "balance" );
    elemRoot -> AddChild ( elemForm2 );


    addGotoTag(elemRoot, elemForm2, "#root");
}

void CreateAuthVXML :: genFormExit( PXMLElement * elemRoot )
{
    PXMLElement * elemForm = new PXMLElement ( elemRoot, "form3" );
    elemForm -> SetAttribute ( "id", "balance" );
    elemRoot -> AddChild ( elemForm );

}

void  CreateAuthVXML :: genEnXml()
{
	PXMLElement * elem = new PXMLElement ( 0, "vxml" );
	elem -> SetAttribute ( "version", "1.0" );
	xml.SetRootElement ( elem );


    genMenu( elem );
    genFormTransferRinging( elem );
    genFormToDeposit( elem );
/*
    addVarTag( elem, elem, "useracc", m_account );
    addVarTag( elem, elem, "userpin", m_password );

    DigitsToString ds(m_digits);

	{
		char strCost[255];
		sprintf( strCost, "./sound/en_you_have.au.wav" );
		addPromptAndAudioData( elem, elemForm, strCost );
	}

	ss :: string strDigits;
	if ( ds.getThousands() )
	{
		char strCost[255];
		int iDigits = ds.getThousands();
		if( iDigits > 100 )
		{
			iDigits = iDigits / 100;
			strDigits = getEnDigit( iDigits );

			sprintf( strCost, "./sound/en_%s.au.wav", strDigits.c_str() );
			addPromptAndAudioData( elem, elemForm, strCost );
			addPromptAndAudioData( elem, elemForm, "./sound/en_hundred.au.wav" );

			iDigits = ds.getThousands() - iDigits * 100;
		}

		strDigits = getEnDigit( iDigits );
		sprintf( strCost, "./sound/en_%s.au.wav", strDigits.c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
		addPromptAndAudioData( elem, elemForm, "./sound/en_thousand.au.wav" );
	}
	if ( ds.getHundreds() )
	{
		char strCost[255];
		sprintf( strCost, "./sound/en_%s.au.wav", getEnDigit( ds.getHundreds() ).c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
		addPromptAndAudioData( elem, elemForm, "./sound/en_hundred.au.wav" );

	}
	if ( ds.getUnits() )
	{
        getEnFirstHandred(elem, elemForm, ds.getUnits());
	}

    if(int(m_digits) > 0 )
        addPromptAndAudioData( elem, elemForm, "./sound/en_dollars.au.wav" );
    if ( ds.getCents() )
    {
        getEnFirstHandred(elem, elemForm, ds.getCents());
        addPromptAndAudioData( elem, elemForm, "./sound/en_cents.au.wav" );
    }

 	{
		char strAccount[255];

//		sprintf( strAccount, "./authenticated.vxml?account=%s", m_account.c_str() );
		sprintf( strAccount, "authenticated" );
//		sprintf( strAccount, "#authenticated" );
		addGotoTag(elem, elemForm, strAccount);
	}
*/
}
///====================================================================================
void  CreateAuthVXML :: genRuXml()
{
	PXMLElement * elem = new PXMLElement ( 0, "vxml" );
	elem -> SetAttribute ( "version", "1.0" );
	xml.SetRootElement ( elem );

    PXMLElement * elemForm = new PXMLElement ( elem, "form" );
	elemForm -> SetAttribute ( "id", "balance" );

	elem -> AddChild ( elemForm );

    addVarTag( elem, elemForm, "useracc", m_account );
    addVarTag( elem, elemForm, "userpin", m_password );

    DigitsToString ds(m_digits);
	{
		char strCost[255];
		sprintf( strCost, "./sound/ru_you_have.au.wav" );
		addPromptAndAudioData( elem, elemForm, strCost );
	}

	if ( ds.getThousands() )
	{
		int thousands = ds.getThousands();
		char strCost[255];

		if ( thousands >= 100 )
		{
			int hundred = thousands - ((thousands / 100) * 100 );
			thousands -= hundred;
			sprintf( strCost, "./sound/ru_%d00.au.wav", hundred );
			addPromptAndAudioData( elem, elemForm, strCost );
		}

		if ( thousands == 100 )
		{
			addPromptAndAudioData( elem, elemForm, "./sound/ru_100.au.wav" );
		}
		else
		{
			addPromptAndAudioData( elem, elemForm, getRuFileDigits(thousands, "thousand").c_str() );
			addPromptAndAudioData( elem, elemForm, getRuFileUnits( thousands, "thousand" ).c_str() );

		}
	}
	if ( ds.getHundreds() )
	{
		char strCost[255];
		sprintf( strCost, "./sound/ru_%d00.au.wav", ds.getHundreds() );
		addPromptAndAudioData( elem, elemForm, strCost );
	}
	if ( ds.getUnits() )
	{
		char strCost[255];
		sprintf( strCost, "./sound/ru_%d.au.wav", ds.getUnits() );

		addPromptAndAudioData( elem, elemForm, getRuFileDigits(ds.getUnits(), "dollar").c_str() );
		addPromptAndAudioData( elem, elemForm, getRuFileUnits(ds.getUnits(), "dollar" ).c_str() );
	}
	{
        char strAccount[255];

//		sprintf( strAccount, "./authenticated.vxml?account=%s", m_account.c_str() );
        sprintf( strAccount, "authenticated" );

		addGotoTag(elem, elemForm, strAccount);
	}
}
///====================================================================================
void CreateAuthVXML :: genVXml()
{
	PSYSTEMLOG(Info, "CreateAuthVXML :: genVXml");
	if( m_lang == "ru" )
	{
		genRuXml();
	}
	else // if ( m_lang == "en" )
	{
		genEnXml();
	}
}
/*
void CreateVXML :: genVXml( const ss :: string& lang )
{
	PXMLElement * elem = new PXMLElement ( 0, "vxml" );
	elem -> SetAttribute ( "version", "1.0" );
	xml.SetRootElement ( elem );
	PXMLElement * elemForm = new PXMLElement ( elem, "form" );
	elem -> AddChild ( elemForm );
	DigitsToString ds(m_digits);

	ss :: string strDigits;

	{
		char strCost[255];
		sprintf( strCost, "./sound/%s_you_have.au.wav", lang.c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
	}

	if ( ds.getThousands() )
	{
		char strCost[255];
		int iDigits = ds.getThousands();
		if( iDigits > 100 )
		{
			iDigits = iDigits / 100;
			strDigits = getEnDigit( iDigits );

			sprintf( strCost, "./sound/%s_%s.au.wav", lang.c_str(), strDigits.c_str() );
			addPromptAndAudioData( elem, elemForm, strCost );
			sprintf( strCost, "./sound/%s_hundred.au.wav", lang.c_str() );
			addPromptAndAudioData( elem, elemForm, strCost );

			iDigits = ds.getThousands() - iDigits * 100;
		}

		strDigits = getEnDigit( iDigits );
		sprintf( strCost, "./sound/%s_%s.au.wav", lang.c_str(), strDigits.c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
		sprintf( strCost, "./sound/%s_thousand.au.wav", lang.c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
	}
	if ( ds.getHundreds() )
	{
		char strCost[255];
		sprintf( strCost, "./sound/%s_%s.au.wav", lang.c_str(), getEnDigit( ds.getHundreds() ).c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
		sprintf( strCost, "./sound/%s_hundred.au.wav", lang.c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
	}
	if ( ds.getUnits() )
	{
		char strCost[255];
		sprintf( strCost, "./sound/%s_%s.au.wav", lang.c_str(), getEnDigit( ds.getUnits() ).c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
		sprintf( strCost, "./sound/%s_dollar.au.wav", lang.c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
	}
	if ( ds.getCents() )
	{
		char strCost[255];
		sprintf( strCost, "./sound/%s_%s.au.wav", lang.c_str(), getEnDigit( ds.getUnits() ).c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
		sprintf( strCost, "./sound/%s_cent.au.wav", lang.c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
	}
}
*/
///====================================================================================
CreateSelectLangVXML :: CreateSelectLangVXML( int retCode, const ss :: string& lang ) :
	CreateVXML( retCode, lang )
{
}
/*
ss :: string CreateSelectLangVXML :: getVXml()
{
	return "";
}
*/
void CreateSelectLangVXML :: genVXml()
{
	PSYSTEMLOG(Info, "CreateSelectLangVXML :: genVXml");
	PXMLElement * elem = new PXMLElement ( 0, "vxml" );
	elem -> SetAttribute ( "version", "1.0" );
	xml.SetRootElement ( elem );
	PXMLElement * elemForm = new PXMLElement ( elem, "form" );
	elemForm -> SetAttribute ( "id", "lang" );
	elem -> AddChild ( elemForm );

	{
		char strCost[255];
		sprintf( strCost, "./sound/%s_bal_tran_success.au.wav", m_lang.c_str() );
		addPromptAndAudioData( elem, elemForm, strCost );
	}
	{
		char strAccount[255];

		sprintf( strAccount, "authenticated" );
//		sprintf( strAccount, "#authenticated" );
		addGotoTag(elem, elemForm, strAccount);
	}
}
///====================================================================================
CreateExpectedTimeVXML :: CreateExpectedTimeVXML ( double times, int retCode, const ss :: string& lang ) :
	CreateVXML( retCode, lang ),
    m_times(times)
{
}

void CreateExpectedTimeVXML :: genVXml()
{
    PSYSTEMLOG(Info, "CreateTimeRingingVXML :: genVXml");
    PXMLElement * elem = new PXMLElement ( 0, "vxml" );
    elem -> SetAttribute ( "version", "1.0" );
    xml.SetRootElement ( elem );
    PXMLElement * elemForm = new PXMLElement ( elem, "form" );
    elemForm -> SetAttribute ( "id", "lang" );
    elem -> AddChild ( elemForm );

    {
        char strCost[255];
        sprintf( strCost, "./sound/en_you_have.au.wav" );
        addPromptAndAudioData( elem, elemForm, strCost );
    }

//<<<<<<< .mine
    int iMinuts = int(m_times/60);
    int iSeconds = int(m_times - iMinuts*60);
        PSYSTEMLOG(Info, "genVXml: m_times: = " << m_times << "; " << iMinuts << " minutes " << iSeconds << " seconds" );
        if ( iMinuts != 0 )
        {
            char strCost[255];
            if(iMinuts == 1)
            {
                sprintf( strCost, "./sound/%s_one.au.wav", m_lang.c_str() );
                addPromptAndAudioData( elem, elemForm, strCost );
                sprintf( strCost, "./sound/%s_minute.au.wav", m_lang.c_str() );
                addPromptAndAudioData( elem, elemForm, strCost );
            }
            else
            {
/**********************/
                int iDigits = iMinuts / 1000;
                if ( iMinuts >= 1000 )
                {
                    char strCost[255];
                    int iThousands = iMinuts / 1000;
                    if( iDigits > 100 )
                    {
                        iDigits = iDigits / 100;
                        ss :: string strDigits = getEnDigit( iDigits );

                        sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(), strDigits.c_str() );
                        addPromptAndAudioData( elem, elemForm, strCost );
                        sprintf( strCost, "./sound/%s_hundred.au.wav", m_lang.c_str() );
                        addPromptAndAudioData( elem, elemForm, strCost );

                        iDigits = iThousands - iDigits * 100;
                    }

                    ss :: string strDigits = getEnDigit( iDigits );
                    sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(), strDigits.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                    sprintf( strCost, "./sound/%s_thousand.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                }
                int iHundrDid = iMinuts - (iMinuts / 1000) * 1000;
                if ( iHundrDid >= 100 )
                {
                    char strCost[255];
                    int hundrd = ( iHundrDid/100 );
                    PSYSTEMLOG(Info, "iHundrDid = " << iHundrDid << "; hundrd = " << hundrd );
                    sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(), getEnDigit( hundrd ).c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                    sprintf( strCost, "./sound/%s_hundred.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                }
/**********************/
                int ostatok = iMinuts - (iMinuts / 1000) * 1000 - (iHundrDid/100)*100;
                char strCost[255];
                if(ostatok)
                {
                    getEnFirstHandred( elem, elemForm, ostatok );
/*
                    sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(),
                         getEnFirstHandred( elem, elemForm, ostatok ).c_str() );*/
                    //addPromptAndAudioData( elem, elemForm, strCost );
                }
                sprintf( strCost, "./sound/%s_minutes.au.wav", m_lang.c_str() );
                addPromptAndAudioData( elem, elemForm, strCost );
            }
        }
        else
        {
            char strCost[255];
            sprintf( strCost, "./sound/%s_zero.au.wav", m_lang.c_str() );
            addPromptAndAudioData( elem, elemForm, strCost );
            sprintf( strCost, "./sound/%s_minutes.au.wav", m_lang.c_str() );
            addPromptAndAudioData( elem, elemForm, strCost );
        }

        { // Seconds
            if ( iSeconds != 0 )
            {
                char strCost[255];
                if(iSeconds == 1)
                {
                    sprintf( strCost, "./sound/%s_one.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                    sprintf( strCost, "./sound/%s_second.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                }
                else
                {
                    char strCost[255];
                    getEnFirstHandred( elem, elemForm, iSeconds );
/*
                    ss :: string seconds = getEnDigit( iSeconds ).c_str();
                    sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(), getEnDigit( iSeconds ).c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
*/
                    sprintf( strCost, "./sound/%s_seconds.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                }
            }
        }
    {
//      char strAccount[255];
//      sprintf( strAccount, "dial.vxml" );
//      addGotoTag(elem, elemForm, strAccount);
//		sprintf( strAccount, "#authenticated" );

        PXMLElement * elemTransfer = new PXMLElement ( elem, "transfer" );

        elemTransfer -> SetAttribute ( "name", "test" );
        elemTransfer -> SetAttribute ( "destexpr", "TransferNumber" );
        elemTransfer -> SetAttribute ( "useracc", "useracc" );
        elemTransfer -> SetAttribute ( "userpin", "userpin" );

        elemForm -> AddChild ( elemTransfer );
        PSYSTEMLOG(Info, "genVXml: transfer" );
    }
}

/*
=======
    int iMinuts = int(m_times/60);
    int iSeconds = int(m_times - iMinuts*60);
        PSYSTEMLOG(Info, "genVXml: m_times: = " << m_times << "; " << iMinuts << " minuts " << iSeconds << " seconds" );
        if ( iMinuts != 0 )
        {
            char strCost[255];
            if(iMinuts == 1)
            {
                sprintf( strCost, "./sound/%s_one.au.wav", m_lang.c_str() );
                addPromptAndAudioData( elem, elemForm, strCost );
                sprintf( strCost, "./sound/%s_minute.au.wav", m_lang.c_str() );
                addPromptAndAudioData( elem, elemForm, strCost );
            }
            else
            {
//////////////////////////
                int iDigits = iMinuts / 1000;
                if ( iMinuts >= 1000 )
                {
                    char strCost[255];
                    int iThousands = iMinuts / 1000;
                    if( iDigits > 100 )
                    {
                        iDigits = iDigits / 100;
                        ss :: string strDigits = getEnDigit( iDigits );

                        sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(), strDigits.c_str() );
                        addPromptAndAudioData( elem, elemForm, strCost );
                        sprintf( strCost, "./sound/%s_hundred.au.wav", m_lang.c_str() );
                        addPromptAndAudioData( elem, elemForm, strCost );

                        iDigits = iThousands - iDigits * 100;
                    }

                    ss :: string strDigits = getEnDigit( iDigits );
                    sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(), strDigits.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                    sprintf( strCost, "./sound/%s_thousand.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                }
                int iHundrDid = iMinuts - (iMinuts / 1000) * 1000;
                if ( iHundrDid >= 100 )
                {
                    char strCost[255];
                    int hundrd = ( iHundrDid/100 );
                    PSYSTEMLOG(Info, "iHundrDid = " << iHundrDid << "; hundrd = " << hundrd );
                    sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(), getEnDigit( hundrd ).c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                    sprintf( strCost, "./sound/%s_hundred.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                }
/////////////////////
                char strCost[255];
                sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(),
                         getEnDigit( iMinuts - (iMinuts / 1000) * 1000 - (iHundrDid/100)*100 ).c_str() );
                addPromptAndAudioData( elem, elemForm, strCost );
                sprintf( strCost, "./sound/%s_minuts.au.wav", m_lang.c_str() );
                addPromptAndAudioData( elem, elemForm, strCost );
            }
        }

        { // Seconds
            if ( iSeconds != 0 )
            {
                char strCost[255];
                if(iSeconds == 1)
                {
                    sprintf( strCost, "./sound/%s_one.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                    sprintf( strCost, "./sound/%s_second.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                }
                else if(iSeconds != 0)
                {
                    char strCost[255];
                    sprintf( strCost, "./sound/%s_%s.au.wav", m_lang.c_str(), getEnDigit( iSeconds ).c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                    sprintf( strCost, "./sound/%s_seconds.au.wav", m_lang.c_str() );
                    addPromptAndAudioData( elem, elemForm, strCost );
                }
            }
        }
    {
//      char strAccount[255];
//      sprintf( strAccount, "dial.vxml" );
//      addGotoTag(elem, elemForm, strAccount);
//		sprintf( strAccount, "#authenticated" );

        PXMLElement * elemTransfer = new PXMLElement ( elem, "transfer" );

        elemTransfer -> SetAttribute ( "name", "test" );
        elemTransfer -> SetAttribute ( "destexpr", "TransferNumber" );
        elemTransfer -> SetAttribute ( "useracc", "useracc" );
        elemTransfer -> SetAttribute ( "userpin", "userpin" );

        elemForm -> AddChild ( elemTransfer );
        PSYSTEMLOG(Info, "genVXml: transfer" );
    }
}


>>>>>>> .r962
*/

