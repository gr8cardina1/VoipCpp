#ifndef SDPGRAMMARS_HPP_
#define SDPGRAMMARS_HPP_
#pragma interface

namespace SDP {
const unsigned sdp_grammar_ipv4address_id = 2;
const unsigned sdp_grammar_information_field_id = 4;
const unsigned sdp_grammar_uri_field_id = 5;
const unsigned sdp_grammar_email_fields_id = 6;
const unsigned sdp_grammar_phone_fields_id = 8;
const unsigned sdp_grammar_connection_field_id = 10;
const unsigned sdp_grammar_bandwidth_fields_id = 11;
const unsigned sdp_grammar_repeat_fields_id = 13;
const unsigned sdp_grammar_zone_adjustments_id = 15;
const unsigned sdp_grammar_key_field_id = 16;
const unsigned sdp_grammar_attribute_fields_id = 17;
const unsigned sdp_grammar_integer_id = 19;

RightParser :: result_t parseSdp ( const ss :: string :: const_iterator & ib, const ss :: string :: const_iterator & ie );

}
#endif /*SDPGRAMMARS_HPP_*/
