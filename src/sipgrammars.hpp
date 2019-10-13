#ifndef SIPGRAMMARS_HPP_
#define SIPGRAMMARS_HPP_
#pragma interface

namespace SIP2 {

const unsigned uri_grammar_user_id = 4;
const unsigned uri_grammar_password_id = 5;
const unsigned uri_grammar_host_id = 6;
const unsigned uri_grammar_port_id = 7;
const unsigned uri_grammar_uriParameter_id = 8;
const unsigned uri_grammar_header_id = 9;

const unsigned fromtocontactheader_grammar_uri_id = 1;

const unsigned retryAfter_grammar_comment_id = 1;

const unsigned via_grammar_genericParam_id = 1;

const unsigned recordRoute_grammar_uri_id = 1;

const unsigned sip_grammar_requestLine_id = 1;
const unsigned sip_grammar_statusLine_id = 2;
const unsigned sip_grammar_messageHeader_id = 3;

RightParser :: result_t parseUri ( const ss :: string & s );
RightParser :: result_t parseFromToContactHeader ( const ss :: string & s );
RightParser :: result_t parseDigestChallenge ( const ss :: string & s );
RightParser :: result_t parseVia ( const ss :: string & s );
RightParser :: result_t parseSip ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie );
RightParser :: result_t parseAuthenticationInfo ( const ss :: string & s );
RightParser :: result_t parseRetryAfter ( const ss :: string & s );
RightParser :: result_t parseRecordRoute ( const ss :: string & s );
RightParser :: result_t parseWarning ( const ss :: string & s );
RightParser :: result_t parseSessionExpires ( const ss :: string & s );
RightParser :: result_t parseSupported ( const ss :: string & s );
RightParser :: result_t parseRequire ( const ss :: string & s );
RightParser :: result_t parseContentType ( const ss :: string & s );
RightParser :: result_t parseAccept ( const ss :: string & s );
RightParser :: result_t parseContentDisposition ( const ss :: string & s );

}
#endif /*SIPGRAMMARS_HPP_*/
