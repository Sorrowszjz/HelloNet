#ifndef __HNET_M_FORMAT_S_HTTPPARSER__
#define __HNET_M_FORMAT_S_HTTPPARSER__

#include "../m_core/m_core.hpp"
/*********************************************************************/
//http_parser
/*********************************************************************/


#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)       (sizeof(a) / sizeof((a)[0]))
#endif
#ifndef BIT_AT
#define BIT_AT(a, i)                                                 \
   (!!((unsigned int) (a)[(unsigned int) (i) >> 3] &                 \
   (1 << ((unsigned int) (i) & 7))))
#endif
#ifndef ELEM_AT
#define ELEM_AT(a, i, v)    ((unsigned int) (i) < ARRAY_SIZE(a) ? (a)[(i)] : (v))
#endif
#ifndef LOWER
#define LOWER(c)            (unsigned char)(c | 0x20)
#endif
#ifndef IS_ALPHA
#define IS_ALPHA(c)         (LOWER(c) >= 'a' && LOWER(c) <= 'z')
#endif
#ifndef IS_NUM
#define IS_NUM(c)           ((c) >= '0' && (c) <= '9')
#endif
#ifndef IS_NUM1TO9
#define IS_NUM1TO9(c)       ((c) >= '1' && (c) <= '9')
#endif    
#ifndef IS_ALPHANUM
#define IS_ALPHANUM(c)      (IS_ALPHA(c) || IS_NUM(c))
#endif
#ifndef IS_HEX
#define IS_HEX(c)           (IS_NUM(c) || (LOWER(c) >= 'a' && LOWER(c) <= 'f'))
#endif
#ifndef IS_MARK
#define IS_MARK(c)          ((c) == '-' || (c) == '_' || (c) == '.' || \
                            (c) == '!' || (c) == '~' || (c) == '*' || (c) == '\'' || (c) == '(' || \
                            (c) == ')')
#endif
#ifndef IS_USERINFO_CHAR
#define IS_USERINFO_CHAR(c) (IS_ALPHANUM(c) || IS_MARK(c) || (c) == '%' || \
  (c) == ';' || (c) == ':' || (c) == '&' || (c) == '=' || (c) == '+' || \
  (c) == '$' || (c) == ',')
#endif
#ifndef IS_HOST_CHAR
#define IS_HOST_CHAR(c)     (IS_ALPHANUM(c) || (c) == '.' || (c) == '-')
#endif
#ifndef HN_CR
#define HN_CR    '\r'
#endif
#ifndef HN_LF
#define HN_LF    '\n'
#endif


/*********************************************************************/
//httptable
/*********************************************************************/
static const uint8_t HN_NORMAL_URL_CHAR[32] = {
    /*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
    0 | 0 | 0 | 0 | 0 | 0 | 0 | 0,
    /*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
    0 | 0 | 0 | 0 | 0 | 0 | 0 | 0,
    /*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
    0 | 0 | 0 | 0 | 0 | 0 | 0 | 0,
    /*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
    0 | 0 | 0 | 0 | 0 | 0 | 0 | 0,
    /*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
    0 | 2 | 4 | 0 | 16 | 32 | 64 | 128,
    /*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 0,
    /*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 128,
    /* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
    1 | 2 | 4 | 8 | 16 | 32 | 64 | 0, };
#ifndef HN_IS_URL_CHAR
#define HN_IS_URL_CHAR(c)      (BIT_AT(HN_NORMAL_URL_CHAR, (unsigned char)c))
#endif
/**
* Verify that a char is a valid visible (printable) US-ASCII
* character or %x80-FF
**/
#ifndef HN_IS_HEADER_CHAR
#define HN_IS_HEADER_CHAR(ch)  (ch == HN_CR || ch == HN_LF || ch == 9 || ((unsigned char)ch > 31 && ch != 127))
#endif

/* Tokens as defined by rfc 2616. Also lowercases them.
*        token       = 1*<any CHAR except CTLs or separators>
*     separators     = "(" | ")" | "<" | ">" | "@"
*                    | "," | ";" | ":" | "\" | <">
*                    | "/" | "[" | "]" | "?" | "="
*                    | "{" | "}" | SP | HT
*/
static const char HN_RFC2616_TOKENS[256] = {
    /*   0 nul    1 soh    2 stx    3 etx    4 eot    5 enq    6 ack    7 bel  */
    0, 0, 0, 0, 0, 0, 0, 0,
    /*   8 bs     9 ht    10 nl    11 vt    12 np    13 cr    14 so    15 si   */
    0, 0, 0, 0, 0, 0, 0, 0,
    /*  16 dle   17 dc1   18 dc2   19 dc3   20 dc4   21 nak   22 syn   23 etb */
    0, 0, 0, 0, 0, 0, 0, 0,
    /*  24 can   25 em    26 sub   27 esc   28 fs    29 gs    30 rs    31 us  */
    0, 0, 0, 0, 0, 0, 0, 0,
    /*  32 sp    33  !    34  "    35  #    36  $    37  %    38  &    39  '  */
    ' ', '!', 0, '#', '$', '%', '&', '\'',
    /*  40  (    41  )    42  *    43  +    44  ,    45  -    46  .    47  /  */
    0, 0, '*', '+', 0, '-', '.', 0,
    /*  48  0    49  1    50  2    51  3    52  4    53  5    54  6    55  7  */
    '0', '1', '2', '3', '4', '5', '6', '7',
    /*  56  8    57  9    58  :    59  ;    60  <    61  =    62  >    63  ?  */
    '8', '9', 0, 0, 0, 0, 0, 0,
    /*  64  @    65  A    66  B    67  C    68  D    69  E    70  F    71  G  */
    0, 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    /*  72  H    73  I    74  J    75  K    76  L    77  M    78  N    79  O  */
    'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    /*  80  P    81  Q    82  R    83  S    84  T    85  U    86  V    87  W  */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
    /*  88  X    89  Y    90  Z    91  [    92  \    93  ]    94  ^    95  _  */
    'x', 'y', 'z', 0, 0, 0, '^', '_',
    /*  96  `    97  a    98  b    99  c   100  d   101  e   102  f   103  g  */
    '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g',
    /* 104  h   105  i   106  j   107  k   108  l   109  m   110  n   111  o  */
    'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
    /* 112  p   113  q   114  r   115  s   116  t   117  u   118  v   119  w  */
    'p', 'q', 'r', 's', 't', 'u', 'v', 'w',
    /* 120  x   121  y   122  z   123  {   124  |   125  }   126  ~   127 del */
    'x', 'y', 'z', 0, '|', 0, '~', 0 };

/* Macros for character classes; depends on strict-mode  */
#ifndef HN_STRICT_TOKEN
#define HN_STRICT_TOKEN(c)     ((c == ' ') ? 0 : HN_RFC2616_TOKENS[(unsigned char)c])
#endif

/** HN_NUMBER_UNHEX
*/
static const int8_t HN_NUMBER_UNHEX[256] =
{ -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, -1, -1, -1, -1
, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1
, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};
#define HNHTTP_PROXY_CONNECTION     "proxy-connection"
#define HNHTTP_CONNECTION           "connection"
#define HNHTTP_CONTENT_LENGTH       "content-length"
#define HNHTTP_TRANSFER_ENCODING    "transfer-encoding"
#define HNHTTP_UPGRADE              "upgrade"
#define HNHTTP_CHUNKED              "chunked"
#define HNHTTP_KEEP_ALIVE           "keep-alive"
#define HNHTTP_CLOSE                "close"


/**
*@brief        全局静态数据
*/
class CHNStaticDeclare : CHNNoCopyable
{
private:
    explicit CHNStaticDeclare()
    {
#define HN_HTTP_DELETE          0            
#define HN_HTTP_GET             1            
#define HN_HTTP_HEAD            2            
#define HN_HTTP_POST            3            
#define HN_HTTP_PUT             4                
#define HN_HTTP_CONNECT         5            
#define HN_HTTP_OPTIONS         6            
#define HN_HTTP_TRACE           7                
#define HN_HTTP_COPY            8            
#define HN_HTTP_LOCK            9            
#define HN_HTTP_MKCOL           10           
#define HN_HTTP_MOVE            11           
#define HN_HTTP_PROPFIND        12           
#define HN_HTTP_PROPPATCH       13           
#define HN_HTTP_SEARCH          14           
#define HN_HTTP_UNLOCK          15           
#define HN_HTTP_BIND            16           
#define HN_HTTP_REBIND          17           
#define HN_HTTP_UNBIND          18           
#define HN_HTTP_ACL             19               
#define HN_HTTP_REPORT          20           
#define HN_HTTP_MKACTIVITY      21           
#define HN_HTTP_CHECKOUT        22           
#define HN_HTTP_MERGE           23               
#define HN_HTTP_MSEARCH         24           
#define HN_HTTP_NOTIFY          25           
#define HN_HTTP_SUBSCRIBE       26           
#define HN_HTTP_UNSUBSCRIBE     27               
#define HN_HTTP_PATCH           28           
#define HN_HTTP_PURGE           29               
#define HN_HTTP_MKCALENDAR      30            
#define HN_HTTP_LINK            31           
#define HN_HTTP_UNLINK          32               
#define HN_HTTP_SOURCE          33 
        m_httpmethod[0] = "DELETE";
        m_httpmethod[1] = "GET";
        m_httpmethod[2] = "HEAD";
        m_httpmethod[3] = "POST";
        m_httpmethod[4] = "PUT";
        m_httpmethod[5] = "CONNECT";
        m_httpmethod[6] = "OPTIONS";
        m_httpmethod[7] = "TRACE";
        m_httpmethod[8] = "COPY";
        m_httpmethod[9] = "LOCK";
        m_httpmethod[10] = "MKCOL";
        m_httpmethod[11] = "MOVE";
        m_httpmethod[12] = "PROPFIND";
        m_httpmethod[13] = "PROPPATCH";
        m_httpmethod[14] = "SEARCH";
        m_httpmethod[15] = "UNLOCK";
        m_httpmethod[16] = "BIND";
        m_httpmethod[17] = "REBIND";
        m_httpmethod[18] = "UNBIND";
        m_httpmethod[19] = "ACL";
        m_httpmethod[20] = "REPORT";
        m_httpmethod[21] = "MKACTIVITY";
        m_httpmethod[22] = "CHECKOUT";
        m_httpmethod[23] = "MERGE";
        m_httpmethod[24] = "M-SEARCH";
        m_httpmethod[25] = "NOTIFY";
        m_httpmethod[26] = "SUBSCRIBE";
        m_httpmethod[27] = "UNSUBSCRIBE";
        m_httpmethod[28] = "PATCH";
        m_httpmethod[29] = "PURGE";
        m_httpmethod[30] = "MKCALENDAR";
        m_httpmethod[31] = "LINK";
        m_httpmethod[32] = "UNLINK";
        m_httpmethod[33] = "SOURCE";

        m_httpstatuscode[100] = "Continue";
        m_httpstatuscode[101] = "Switching Protocols";
        m_httpstatuscode[102] = "Processing";
        m_httpstatuscode[200] = "OK";
        m_httpstatuscode[201] = "Created";
        m_httpstatuscode[202] = "Accepted";
        m_httpstatuscode[203] = "Non-Authoritative Information";
        m_httpstatuscode[204] = "No Content";
        m_httpstatuscode[205] = "Reset Content";
        m_httpstatuscode[206] = "Partial Content";
        m_httpstatuscode[207] = "Multi-Status";
        m_httpstatuscode[208] = "Already Reported";
        m_httpstatuscode[226] = "IM Used";
        m_httpstatuscode[300] = "Multiple Choices";
        m_httpstatuscode[301] = "Moved Permanently";
        m_httpstatuscode[302] = "Found";
        m_httpstatuscode[303] = "See Other";
        m_httpstatuscode[304] = "Not Modified";
        m_httpstatuscode[305] = "Use Proxy";
        m_httpstatuscode[307] = "Temporary Redirect";
        m_httpstatuscode[308] = "Permanent Redirect";
        m_httpstatuscode[400] = "Bad Request";
        m_httpstatuscode[401] = "Unauthorized";
        m_httpstatuscode[402] = "Payment Required";
        m_httpstatuscode[403] = "Forbidden";
        m_httpstatuscode[404] = "Not Found";
        m_httpstatuscode[405] = "Method Not Allowed";
        m_httpstatuscode[406] = "Not Acceptable";
        m_httpstatuscode[407] = "Proxy Authentication Required";
        m_httpstatuscode[408] = "Request Timeout";
        m_httpstatuscode[409] = "Conflict";
        m_httpstatuscode[410] = "Gone";
        m_httpstatuscode[411] = "Length Required";
        m_httpstatuscode[412] = "Precondition Failed";
        m_httpstatuscode[413] = "Payload Too Large";
        m_httpstatuscode[414] = "URI Too Long";
        m_httpstatuscode[415] = "Unsupported Media Type";
        m_httpstatuscode[416] = "Range Not Satisfiable";
        m_httpstatuscode[417] = "Expectation Failed";
        m_httpstatuscode[421] = "Misdirected Request";
        m_httpstatuscode[422] = "Unprocessable Entity";
        m_httpstatuscode[423] = "Locked";
        m_httpstatuscode[424] = "Failed Dependency";
        m_httpstatuscode[426] = "Upgrade Required";
        m_httpstatuscode[428] = "Precondition Required";
        m_httpstatuscode[429] = "Too Many Requests";
        m_httpstatuscode[431] = "Request Header Fields Too Large";
        m_httpstatuscode[451] = "Unavailable For Legal Reasons";
        m_httpstatuscode[500] = "Internal Server Error";
        m_httpstatuscode[501] = "Not Implemented";
        m_httpstatuscode[502] = "Bad Gateway";
        m_httpstatuscode[503] = "Service Unavailable";
        m_httpstatuscode[504] = "Gateway Timeout";
        m_httpstatuscode[505] = "HTTP Version Not Supported";
        m_httpstatuscode[506] = "Variant Also Negotiates";
        m_httpstatuscode[507] = "Insufficient Storage";
        m_httpstatuscode[508] = "Loop Detected";
        m_httpstatuscode[510] = "Not Extended";
        m_httpstatuscode[511] = "Network Authentication Required";

        m_httpmimetypes["001"] = "application/x-001";
        m_httpmimetypes["301"] = "application/x-301";
        m_httpmimetypes["323"] = "text/h323";
        m_httpmimetypes["906"] = "application/x-906";
        m_httpmimetypes["907"] = "drawing/907";
        m_httpmimetypes["a11"] = "application/x-a11";
        m_httpmimetypes["acp"] = "audio/x-mei-aac";
        m_httpmimetypes["ai"] = "application/postscript";
        m_httpmimetypes["aif"] = "audio/aiff";
        m_httpmimetypes["aifc"] = "audio/aiff";
        m_httpmimetypes["aiff"] = "audio/aiff";
        m_httpmimetypes["anv"] = "application/x-anv";
        m_httpmimetypes["asa"] = "text/asa";
        m_httpmimetypes["asf"] = "video/x-ms-asf";
        m_httpmimetypes["asp"] = "text/asp";
        m_httpmimetypes["asx"] = "video/x-ms-asf";
        m_httpmimetypes["au"] = "audio/basic";
        m_httpmimetypes["avi"] = "video/avi";
        m_httpmimetypes["awf"] = "application/vnd.adobe.workflow";
        m_httpmimetypes["biz"] = "text/xml";
        m_httpmimetypes["bmp"] = "application/x-bmp";
        m_httpmimetypes["bot"] = "application/x-bot";
        m_httpmimetypes["c4t"] = "application/x-c4t";
        m_httpmimetypes["c90"] = "application/x-c90";
        m_httpmimetypes["cal"] = "application/x-cals";
        m_httpmimetypes["cat"] = "application/vnd.ms-pki.seccat";
        m_httpmimetypes["cdf"] = "application/x-netcdf";
        m_httpmimetypes["cdr"] = "application/x-cdr";
        m_httpmimetypes["cel"] = "application/x-cel";
        m_httpmimetypes["cer"] = "application/x-x509-ca-cert";
        m_httpmimetypes["cg4"] = "application/x-g4";
        m_httpmimetypes["cgm"] = "application/x-cgm";
        m_httpmimetypes["cit"] = "application/x-cit";
        m_httpmimetypes["class"] = "java/*";
        m_httpmimetypes["cml"] = "text/xml";
        m_httpmimetypes["cmp"] = "application/x-cmp";
        m_httpmimetypes["cmx"] = "application/x-cmx";
        m_httpmimetypes["cot"] = "application/x-cot";
        m_httpmimetypes["crl"] = "application/pkix-crl";
        m_httpmimetypes["crt"] = "application/x-x509-ca-cert";
        m_httpmimetypes["csi"] = "application/x-csi";
        m_httpmimetypes["css"] = "text/css";
        m_httpmimetypes["cut"] = "application/x-cut";
        m_httpmimetypes["dbf"] = "application/x-dbf";
        m_httpmimetypes["dbm"] = "application/x-dbm";
        m_httpmimetypes["dbx"] = "application/x-dbx";
        m_httpmimetypes["dcd"] = "text/xml";
        m_httpmimetypes["dcx"] = "application/x-dcx";
        m_httpmimetypes["der"] = "application/x-x509-ca-cert";
        m_httpmimetypes["dgn"] = "application/x-dgn";
        m_httpmimetypes["dib"] = "application/x-dib";
        m_httpmimetypes["dll"] = "application/x-msdownload";
        m_httpmimetypes["doc"] = "application/msword";
        m_httpmimetypes["dot"] = "application/msword";
        m_httpmimetypes["drw"] = "application/x-drw";
        m_httpmimetypes["dtd"] = "text/xml";
        m_httpmimetypes["dwf"] = "Model/vnd.dwf";
        m_httpmimetypes["dwf"] = "application/x-dwf";
        m_httpmimetypes["dwg"] = "application/x-dwg";
        m_httpmimetypes["dxb"] = "application/x-dxb";
        m_httpmimetypes["dxf"] = "application/x-dxf";
        m_httpmimetypes["edn"] = "application/vnd.adobe.edn";
        m_httpmimetypes["emf"] = "application/x-emf";
        m_httpmimetypes["eml"] = "message/rfc822";
        m_httpmimetypes["ent"] = "text/xml";
        m_httpmimetypes["epi"] = "application/x-epi";
        m_httpmimetypes["eps"] = "application/x-ps";
        m_httpmimetypes["eps"] = "application/postscript";
        m_httpmimetypes["etd"] = "application/x-ebx";
        m_httpmimetypes["exe"] = "application/x-msdownload";
        m_httpmimetypes["fax"] = "image/fax";
        m_httpmimetypes["fdf"] = "application/vnd.fdf";
        m_httpmimetypes["fif"] = "application/fractals";
        m_httpmimetypes["fo"] = "text/xml";
        m_httpmimetypes["frm"] = "application/x-frm";
        m_httpmimetypes["g4"] = "application/x-g4";
        m_httpmimetypes["gbr"] = "application/x-gbr";
        m_httpmimetypes["gif"] = "image/gif";
        m_httpmimetypes["gl2"] = "application/x-gl2";
        m_httpmimetypes["gp4"] = "application/x-gp4";
        m_httpmimetypes["hgl"] = "application/x-hgl";
        m_httpmimetypes["hmr"] = "application/x-hmr";
        m_httpmimetypes["hpg"] = "application/x-hpgl";
        m_httpmimetypes["hpl"] = "application/x-hpl";
        m_httpmimetypes["hqx"] = "application/mac-binhex40";
        m_httpmimetypes["hrf"] = "application/x-hrf";
        m_httpmimetypes["hta"] = "application/hta";
        m_httpmimetypes["htc"] = "text/x-component";
        m_httpmimetypes["htm"] = "text/html";
        m_httpmimetypes["html"] = "text/html";
        m_httpmimetypes["htt"] = "text/webviewhtml";
        m_httpmimetypes["htx"] = "text/html";
        m_httpmimetypes["icb"] = "application/x-icb";
        m_httpmimetypes["ico"] = "image/x-icon";
        m_httpmimetypes["ico"] = "application/x-ico";
        m_httpmimetypes["iff"] = "application/x-iff";
        m_httpmimetypes["ig4"] = "application/x-g4";
        m_httpmimetypes["igs"] = "application/x-igs";
        m_httpmimetypes["iii"] = "application/x-iphone";
        m_httpmimetypes["img"] = "application/x-img";
        m_httpmimetypes["ins"] = "application/x-internet-signup";
        m_httpmimetypes["isp"] = "application/x-internet-signup";
        m_httpmimetypes["IVF"] = "video/x-ivf";
        m_httpmimetypes["java"] = "java/*";
        m_httpmimetypes["jfif"] = "image/jpeg";
        m_httpmimetypes["jpe"] = "image/jpeg";
        m_httpmimetypes["jpe"] = "application/x-jpe";
        m_httpmimetypes["jpeg"] = "image/jpeg";
        m_httpmimetypes["jpg"] = "image/jpeg";
        m_httpmimetypes["jpg"] = "application/x-jpg";
        m_httpmimetypes["js"] = "application/x-javascript";
        m_httpmimetypes["jsp"] = "text/html";
        m_httpmimetypes["la1"] = "audio/x-liquid-file";
        m_httpmimetypes["lar"] = "application/x-laplayer-reg";
        m_httpmimetypes["latex"] = "application/x-latex";
        m_httpmimetypes["lavs"] = "audio/x-liquid-secure";
        m_httpmimetypes["lbm"] = "application/x-lbm";
        m_httpmimetypes["lmsff"] = "audio/x-la-lms";
        m_httpmimetypes["ls"] = "application/x-javascript";
        m_httpmimetypes["ltr"] = "application/x-ltr";
        m_httpmimetypes["m1v"] = "video/x-mpeg";
        m_httpmimetypes["m2v"] = "video/x-mpeg";
        m_httpmimetypes["m3u"] = "audio/mpegurl";
        m_httpmimetypes["m4e"] = "video/mpeg4";
        m_httpmimetypes["mac"] = "application/x-mac";
        m_httpmimetypes["man"] = "application/x-troff-man";
        m_httpmimetypes["math"] = "text/xml";
        m_httpmimetypes["mdb"] = "application/msaccess";
        m_httpmimetypes["mdb"] = "application/x-mdb";
        m_httpmimetypes["mfp"] = "application/x-shockwave-flash";
        m_httpmimetypes["mht"] = "message/rfc822";
        m_httpmimetypes["mhtml"] = "message/rfc822";
        m_httpmimetypes["mi"] = "application/x-mi";
        m_httpmimetypes["mid"] = "audio/mid";
        m_httpmimetypes["midi"] = "audio/mid";
        m_httpmimetypes["mil"] = "application/x-mil";
        m_httpmimetypes["mml"] = "text/xml";
        m_httpmimetypes["mnd"] = "audio/x-musicnet-download";
        m_httpmimetypes["mns"] = "audio/x-musicnet-stream";
        m_httpmimetypes["mocha"] = "application/x-javascript";
        m_httpmimetypes["movie"] = "video/x-sgi-movie";
        m_httpmimetypes["mp1"] = "audio/mp1";
        m_httpmimetypes["mp2"] = "audio/mp2";
        m_httpmimetypes["mp2v"] = "video/mpeg";
        m_httpmimetypes["mp3"] = "audio/mp3";
        m_httpmimetypes["mp4"] = "video/mpeg4";
        m_httpmimetypes["mpa"] = "video/x-mpg";
        m_httpmimetypes["mpd"] = "application/vnd.ms-project";
        m_httpmimetypes["mpe"] = "video/x-mpeg";
        m_httpmimetypes["mpeg"] = "video/mpg";
        m_httpmimetypes["mpg"] = "video/mpg";
        m_httpmimetypes["mpga"] = "audio/rn-mpeg";
        m_httpmimetypes["mpp"] = "application/vnd.ms-project";
        m_httpmimetypes["mps"] = "video/x-mpeg";
        m_httpmimetypes["mpt"] = "application/vnd.ms-project";
        m_httpmimetypes["mpv"] = "video/mpg";
        m_httpmimetypes["mpv2"] = "video/mpeg";
        m_httpmimetypes["mpw"] = "application/vnd.ms-project";
        m_httpmimetypes["mpx"] = "application/vnd.ms-project";
        m_httpmimetypes["mtx"] = "text/xml";
        m_httpmimetypes["mxp"] = "application/x-mmxp";
        m_httpmimetypes["net"] = "image/pnetvue";
        m_httpmimetypes["nrf"] = "application/x-nrf";
        m_httpmimetypes["nws"] = "message/rfc822";
        m_httpmimetypes["odc"] = "text/x-ms-odc";
        m_httpmimetypes["out"] = "application/x-out";
        m_httpmimetypes["p10"] = "application/pkcs10";
        m_httpmimetypes["p12"] = "application/x-pkcs12";
        m_httpmimetypes["p7b"] = "application/x-pkcs7-certificates";
        m_httpmimetypes["p7c"] = "application/pkcs7-mime";
        m_httpmimetypes["p7m"] = "application/pkcs7-mime";
        m_httpmimetypes["p7r"] = "application/x-pkcs7-certreqresp";
        m_httpmimetypes["p7s"] = "application/pkcs7-signature";
        m_httpmimetypes["pc5"] = "application/x-pc5";
        m_httpmimetypes["pci"] = "application/x-pci";
        m_httpmimetypes["pcl"] = "application/x-pcl";
        m_httpmimetypes["pcx"] = "application/x-pcx";
        m_httpmimetypes["pdf"] = "application/pdf";
        m_httpmimetypes["pdf"] = "application/pdf";
        m_httpmimetypes["pdx"] = "application/vnd.adobe.pdx";
        m_httpmimetypes["pfx"] = "application/x-pkcs12";
        m_httpmimetypes["pgl"] = "application/x-pgl";
        m_httpmimetypes["pic"] = "application/x-pic";
        m_httpmimetypes["pko"] = "application/vnd.ms-pki.pko";
        m_httpmimetypes["pl"] = "application/x-perl";
        m_httpmimetypes["plg"] = "text/html";
        m_httpmimetypes["pls"] = "audio/scpls";
        m_httpmimetypes["plt"] = "application/x-plt";
        m_httpmimetypes["png"] = "image/png";
        m_httpmimetypes["png"] = "application/x-png";
        m_httpmimetypes["pot"] = "application/vnd.ms-powerpoint";
        m_httpmimetypes["ppa"] = "application/vnd.ms-powerpoint";
        m_httpmimetypes["ppm"] = "application/x-ppm";
        m_httpmimetypes["pps"] = "application/vnd.ms-powerpoint";
        m_httpmimetypes["ppt"] = "application/vnd.ms-powerpoint";
        m_httpmimetypes["ppt"] = "application/x-ppt";
        m_httpmimetypes["pr"] = "application/x-pr";
        m_httpmimetypes["prf"] = "application/pics-rules";
        m_httpmimetypes["prn"] = "application/x-prn";
        m_httpmimetypes["prt"] = "application/x-prt";
        m_httpmimetypes["ps"] = "application/x-ps";
        m_httpmimetypes["ps"] = "application/postscript";
        m_httpmimetypes["ptn"] = "application/x-ptn";
        m_httpmimetypes["pwz"] = "application/vnd.ms-powerpoint";
        m_httpmimetypes["r3t"] = "text/vnd.rn-realtext3d";
        m_httpmimetypes["ra"] = "audio/vnd.rn-realaudio";
        m_httpmimetypes["ram"] = "audio/x-pn-realaudio";
        m_httpmimetypes["ras"] = "application/x-ras";
        m_httpmimetypes["rat"] = "application/rat-file";
        m_httpmimetypes["rdf"] = "text/xml";
        m_httpmimetypes["rec"] = "application/vnd.rn-recording";
        m_httpmimetypes["red"] = "application/x-red";
        m_httpmimetypes["rgb"] = "application/x-rgb";
        m_httpmimetypes["rjs"] = "application/vnd.rn-realsystem-rjs";
        m_httpmimetypes["rjt"] = "application/vnd.rn-realsystem-rjt";
        m_httpmimetypes["rlc"] = "application/x-rlc";
        m_httpmimetypes["rle"] = "application/x-rle";
        m_httpmimetypes["rm"] = "application/vnd.rn-realmedia";
        m_httpmimetypes["rmf"] = "application/vnd.adobe.rmf";
        m_httpmimetypes["rmi"] = "audio/mid";
        m_httpmimetypes["rmj"] = "application/vnd.rn-realsystem-rmj";
        m_httpmimetypes["rmm"] = "audio/x-pn-realaudio";
        m_httpmimetypes["rmp"] = "application/vnd.rn-rn_music_package";
        m_httpmimetypes["rms"] = "application/vnd.rn-realmedia-secure";
        m_httpmimetypes["rmvb"] = "application/vnd.rn-realmedia-vbr";
        m_httpmimetypes["rmx"] = "application/vnd.rn-realsystem-rmx";
        m_httpmimetypes["rnx"] = "application/vnd.rn-realplayer";
        m_httpmimetypes["rp"] = "image/vnd.rn-realpix";
        m_httpmimetypes["rpm"] = "audio/x-pn-realaudio-plugin";
        m_httpmimetypes["rsml"] = "application/vnd.rn-rsml";
        m_httpmimetypes["rt"] = "text/vnd.rn-realtext";
        m_httpmimetypes["rtf"] = "application/msword";
        m_httpmimetypes["rtf"] = "application/x-rtf";
        m_httpmimetypes["rv"] = "video/vnd.rn-realvideo";
        m_httpmimetypes["sam"] = "application/x-sam";
        m_httpmimetypes["sat"] = "application/x-sat";
        m_httpmimetypes["sdp"] = "application/sdp";
        m_httpmimetypes["sdw"] = "application/x-sdw";
        m_httpmimetypes["sit"] = "application/x-stuffit";
        m_httpmimetypes["slb"] = "application/x-slb";
        m_httpmimetypes["sld"] = "application/x-sld";
        m_httpmimetypes["slk"] = "drawing/x-slk";
        m_httpmimetypes["smi"] = "application/smil";
        m_httpmimetypes["smil"] = "application/smil";
        m_httpmimetypes["smk"] = "application/x-smk";
        m_httpmimetypes["snd"] = "audio/basic";
        m_httpmimetypes["sol"] = "text/plain";
        m_httpmimetypes["sor"] = "text/plain";
        m_httpmimetypes["spc"] = "application/x-pkcs7-certificates";
        m_httpmimetypes["spl"] = "application/futuresplash";
        m_httpmimetypes["spp"] = "text/xml";
        m_httpmimetypes["ssm"] = "application/streamingmedia";
        m_httpmimetypes["sst"] = "application/vnd.ms-pki.certstore";
        m_httpmimetypes["stl"] = "application/vnd.ms-pki.stl";
        m_httpmimetypes["stm"] = "text/html";
        m_httpmimetypes["sty"] = "application/x-sty";
        m_httpmimetypes["svg"] = "text/xml";
        m_httpmimetypes["swf"] = "application/x-shockwave-flash";
        m_httpmimetypes["tdf"] = "application/x-tdf";
        m_httpmimetypes["tg4"] = "application/x-tg4";
        m_httpmimetypes["tga"] = "application/x-tga";
        m_httpmimetypes["tif"] = "image/tiff";
        m_httpmimetypes["tiff"] = "image/tiff";
        m_httpmimetypes["tld"] = "text/xml";
        m_httpmimetypes["top"] = "drawing/x-top";
        m_httpmimetypes["torrent"] = "application/x-bittorrent";
        m_httpmimetypes["tsd"] = "text/xml";
        m_httpmimetypes["txt"] = "text/plain";
        m_httpmimetypes["uin"] = "application/x-icq";
        m_httpmimetypes["uls"] = "text/iuls";
        m_httpmimetypes["vcf"] = "text/x-vcard";
        m_httpmimetypes["vda"] = "application/x-vda";
        m_httpmimetypes["vdx"] = "application/vnd.visio";
        m_httpmimetypes["vml"] = "text/xml";
        m_httpmimetypes["vpg"] = "application/x-vpeg005";
        m_httpmimetypes["vsd"] = "application/vnd.visio";
        m_httpmimetypes["vsd"] = "application/x-vsd";
        m_httpmimetypes["vss"] = "application/vnd.visio";
        m_httpmimetypes["vst"] = "application/vnd.visio";
        m_httpmimetypes["vst"] = "application/x-vst";
        m_httpmimetypes["vsw"] = "application/vnd.visio";
        m_httpmimetypes["vsx"] = "application/vnd.visio";
        m_httpmimetypes["vtx"] = "application/vnd.visio";
        m_httpmimetypes["vxml"] = "text/xml";
        m_httpmimetypes["wav"] = "audio/wav";
        m_httpmimetypes["wax"] = "audio/x-ms-wax";
        m_httpmimetypes["wb1"] = "application/x-wb1";
        m_httpmimetypes["wb2"] = "application/x-wb2";
        m_httpmimetypes["wb3"] = "application/x-wb3";
        m_httpmimetypes["wbmp"] = "image/vnd.wap.wbmp";
        m_httpmimetypes["wiz"] = "application/msword";
        m_httpmimetypes["wk3"] = "application/x-wk3";
        m_httpmimetypes["wk4"] = "application/x-wk4";
        m_httpmimetypes["wkq"] = "application/x-wkq";
        m_httpmimetypes["wks"] = "application/x-wks";
        m_httpmimetypes["wm"] = "video/x-ms-wm";
        m_httpmimetypes["wma"] = "audio/x-ms-wma";
        m_httpmimetypes["wmd"] = "application/x-ms-wmd";
        m_httpmimetypes["wmf"] = "application/x-wmf";
        m_httpmimetypes["wml"] = "text/vnd.wap.wml";
        m_httpmimetypes["wmv"] = "video/x-ms-wmv";
        m_httpmimetypes["wmx"] = "video/x-ms-wmx";
        m_httpmimetypes["wmz"] = "application/x-ms-wmz";
        m_httpmimetypes["wp6"] = "application/x-wp6";
        m_httpmimetypes["wpd"] = "application/x-wpd";
        m_httpmimetypes["wpg"] = "application/x-wpg";
        m_httpmimetypes["wpl"] = "application/vnd.ms-wpl";
        m_httpmimetypes["wq1"] = "application/x-wq1";
        m_httpmimetypes["wr1"] = "application/x-wr1";
        m_httpmimetypes["wri"] = "application/x-wri";
        m_httpmimetypes["wrk"] = "application/x-wrk";
        m_httpmimetypes["ws"] = "application/x-ws";
        m_httpmimetypes["ws2"] = "application/x-ws";
        m_httpmimetypes["wsc"] = "text/scriptlet";
        m_httpmimetypes["wsdl"] = "text/xml";
        m_httpmimetypes["wvx"] = "video/x-ms-wvx";
        m_httpmimetypes["xdp"] = "application/vnd.adobe.xdp";
        m_httpmimetypes["xdr"] = "text/xml";
        m_httpmimetypes["xfd"] = "application/vnd.adobe.xfd";
        m_httpmimetypes["xfdf"] = "application/vnd.adobe.xfdf";
        m_httpmimetypes["xhtml"] = "text/html";
        m_httpmimetypes["xls"] = "application/vnd.ms-excel";
        m_httpmimetypes["xls"] = "application/x-xls";
        m_httpmimetypes["xlw"] = "application/x-xlw";
        m_httpmimetypes["xml"] = "text/xml";
        m_httpmimetypes["xpl"] = "audio/scpls";
        m_httpmimetypes["xq"] = "text/xml";
        m_httpmimetypes["xql"] = "text/xml";
        m_httpmimetypes["xquery"] = "text/xml";
        m_httpmimetypes["xsd"] = "text/xml";
        m_httpmimetypes["xsl"] = "text/xml";
        m_httpmimetypes["xslt"] = "text/xml";
        m_httpmimetypes["xwd"] = "application/x-xwd";
        m_httpmimetypes["x_b"] = "application/x-x_b";
        m_httpmimetypes["sis"] = "application/vnd.symbian.install";
        m_httpmimetypes["sisx"] = "application/vnd.symbian.install";
        m_httpmimetypes["x_t"] = "application/x-x_t";
        m_httpmimetypes["ipa"] = "application/vnd.iphone";
        m_httpmimetypes["apk"] = "application/vnd.android.package-archive";
        m_httpmimetypes["xap"] = "application/x-silverlight-app";
    }

public:
    /**
    *@brief        单例对象
    */
    static CHNStaticDeclare& obj()
    {
        static CHNStaticDeclare o;
        return o;
    }

    /**
    *@brief        根据http方法index获取http方法名
    *@param        method    [in]    http方法index
    *@return    http方法名，如果没有，默认为<unknown>
    */
    std::string GetHttpMethodStr(unsigned int method)
    {
        auto typeiter = m_httpmethod.find(method);
        if (typeiter == m_httpmethod.end())
        {
            return "<unknown>";
        }
        return (*typeiter).second;
    }

    /**
    *@brief        根据http状态码获取状态描述字符串
    *@param        code    [in]    http状态码
    *@return    状态描述字符串，如果没有，默认为<unknown>
    */
    std::string GetHttpStatusCodeStr(unsigned int code)
    {
        auto typeiter = m_httpstatuscode.find(code);
        if (typeiter == m_httpstatuscode.end())
        {
            return "<unknown>";
        }
        return (*typeiter).second;
    }

    /**
    *@brief        根据文件后缀名获取MimeType
    *@param        ext    [in]    后缀名
    *@return    MimeType，如果没有，默认为application/octet-stream
    */
    std::string GetHttpMimeType(const std::string ext)
    {
        auto typeiter = m_httpmimetypes.find(ext);
        if (typeiter == m_httpmimetypes.end())
        {
            return "application/octet-stream";
        }
        return (*typeiter).second;
    }

public:
    std::unordered_map<unsigned int, std::string> m_httpmethod;        //http方法表
    std::unordered_map<unsigned int, std::string> m_httpstatuscode;    //http状态表
    std::unordered_map<std::string, std::string>  m_httpmimetypes;    //http的Content-Type类型表MineType 
};


/**
*@brief    http_parser库的内容，包装到命名空间避免污染
*/
namespace hnhttpparser
{


    enum http_parser_type { HTTP_REQUEST, HTTP_RESPONSE, HTTP_BOTH };


    /* Flag values for http_parser.flags field */
    enum flags
    {
        F_CHUNKED = 1 << 0
        , F_CONNECTION_KEEP_ALIVE = 1 << 1
        , F_CONNECTION_CLOSE = 1 << 2
        , F_CONNECTION_UPGRADE = 1 << 3
        , F_TRAILING = 1 << 4
        , F_UPGRADE = 1 << 5
        , F_SKIPBODY = 1 << 6
        , F_CONTENTLENGTH = 1 << 7
    };


    enum header_states
    {
        h_general = 0
        , h_C
        , h_CO
        , h_CON

        , h_matching_connection
        , h_matching_proxy_connection
        , h_matching_content_length
        , h_matching_transfer_encoding
        , h_matching_upgrade

        , h_connection
        , h_content_length
        , h_content_length_num
        , h_content_length_ws
        , h_transfer_encoding
        , h_upgrade

        , h_matching_transfer_encoding_chunked
        , h_matching_connection_token_start
        , h_matching_connection_keep_alive
        , h_matching_connection_close
        , h_matching_connection_upgrade
        , h_matching_connection_token

        , h_transfer_encoding_chunked
        , h_connection_keep_alive
        , h_connection_close
        , h_connection_upgrade
    };

    enum http_host_state
    {
        s_http_host_dead = 1
        , s_http_userinfo_start
        , s_http_userinfo
        , s_http_host_start
        , s_http_host_v6_start
        , s_http_host
        , s_http_host_v6
        , s_http_host_v6_end
        , s_http_host_v6_zone_start
        , s_http_host_v6_zone
        , s_http_host_port_start
        , s_http_host_port
    };

    enum state
    {
        s_dead = 1 /* important that this is > 0 */

        , s_start_req_or_res
        , s_res_or_resp_H
        , s_start_res
        , s_res_H
        , s_res_HT
        , s_res_HTT
        , s_res_HTTP
        , s_res_http_major
        , s_res_http_dot
        , s_res_http_minor
        , s_res_http_end
        , s_res_first_status_code
        , s_res_status_code
        , s_res_status_start
        , s_res_status
        , s_res_line_almost_done

        , s_start_req

        , s_req_method
        , s_req_spaces_before_url
        , s_req_schema
        , s_req_schema_slash
        , s_req_schema_slash_slash
        , s_req_server_start
        , s_req_server
        , s_req_server_with_at
        , s_req_path
        , s_req_query_string_start
        , s_req_query_string
        , s_req_fragment_start
        , s_req_fragment
        , s_req_http_start
        , s_req_http_H
        , s_req_http_HT
        , s_req_http_HTT
        , s_req_http_HTTP
        , s_req_http_I
        , s_req_http_IC
        , s_req_http_major
        , s_req_http_dot
        , s_req_http_minor
        , s_req_http_end
        , s_req_line_almost_done

        , s_header_field_start
        , s_header_field
        , s_header_value_discard_ws
        , s_header_value_discard_ws_almost_done
        , s_header_value_discard_lws
        , s_header_value_start
        , s_header_value
        , s_header_value_lws

        , s_header_almost_done

        , s_chunk_size_start
        , s_chunk_size
        , s_chunk_parameters
        , s_chunk_size_almost_done

        , s_headers_almost_done
        , s_headers_done

        /* Important: 's_headers_done' must be the last 'header' state. All
        * states beyond this must be 'body' states. It is used for overflow
        * checking. See the HNHTTP_PARSING_HEADER() macro.
        */

        , s_chunk_data
        , s_chunk_data_almost_done
        , s_chunk_data_done

        , s_body_identity
        , s_body_identity_eof

        , s_message_done
    };

    enum http_parser_url_fields
    {
        UF_SCHEMA = 0
        , UF_HOST = 1
        , UF_PORT = 2
        , UF_PATH = 3
        , UF_QUERY = 4
        , UF_FRAGMENT = 5
        , UF_USERINFO = 6
        , UF_MAX = 7
    };

    /* Result structure for http_parser_parse_url().
    *
    * Callers should index into field_data[] with UF_* values iff field_set
    * has the relevant (1 << UF_*) bit set. As a courtesy to clients (and
    * because we probably have padding left over), we convert any port to
    * a uint16_t.
    */
    typedef struct _http_parser_url {
        uint16_t field_set;           /* Bitmask of (1 << UF_*) values */
        uint16_t port;                /* Converted UF_PORT string */

        struct {
            uint16_t off;               /* Offset into buffer in which field starts */
            uint16_t len;               /* Length of run in buffer */
        } field_data[UF_MAX];
    }http_parser_url;


    typedef struct _http_parser {
        /** PRIVATE **/
        unsigned int type : 2;         /* enum http_parser_type */
        unsigned int flags : 8;        /* F_* values from 'flags' enum; semi-public */
        unsigned int state : 7;        /* enum state from http_parser.c */
        unsigned int header_state : 7; /* enum header_state from http_parser.c */
        unsigned int index : 7;        /* index into current matcher */
        unsigned int lenient_http_headers : 1;

        uint32_t nread;          /* # bytes read in various scenarios */
        uint64_t content_length; /* # bytes in body (0 if no Content-Length header) */

        /** READ-ONLY **/
        unsigned short http_major;
        unsigned short http_minor;
        unsigned int status_code : 16; /* responses only */
        unsigned int method : 8;       /* requests only */
        unsigned int http_errno : 7;

        /* 1 = Upgrade header was present and the parser has exited because of that.
        * 0 = No upgrade header present.
        * Should be checked when http_parser_execute() returns in addition to
        * error checking.
        */
        unsigned int upgrade : 1;

        /** PUBLIC **/
        void *data; /* A pointer to get hook to the "connection" or "socket" object */
    }http_parser;

    typedef int(*http_data_cb) (http_parser*, const char *at, size_t length);
    typedef int(*http_cb) (http_parser*);

    typedef struct _http_parser_settings {
        http_cb      on_message_begin;
        http_data_cb on_url;
        http_data_cb on_status;
        http_data_cb on_header_field;
        http_data_cb on_header_value;
        http_cb      on_headers_complete;
        http_data_cb on_body;
        http_cb      on_message_complete;
        /* When on_chunk_header is called, the current chunk length is stored
        * in parser->content_length.
        */
        http_cb      on_chunk_header;
        http_cb      on_chunk_complete;
    }http_parser_settings;



    /* Map for errno-related constants
    *
    * The provided argument should be a macro that takes 2 arguments.
    */
#define HNHTTP_HTTP_ERRNO_MAP(XX)                                           \
  /* No error */                                                     \
  XX(OK, "success")                                                  \
                                                                     \
  /* Callback-related errors */                                      \
  XX(CB_message_begin, "the on_message_begin callback failed")       \
  XX(CB_url, "the on_url callback failed")                           \
  XX(CB_header_field, "the on_header_field callback failed")         \
  XX(CB_header_value, "the on_header_value callback failed")         \
  XX(CB_headers_complete, "the on_headers_complete callback failed") \
  XX(CB_body, "the on_body callback failed")                         \
  XX(CB_message_complete, "the on_message_complete callback failed") \
  XX(CB_status, "the on_status callback failed")                     \
  XX(CB_chunk_header, "the on_chunk_header callback failed")         \
  XX(CB_chunk_complete, "the on_chunk_complete callback failed")     \
                                                                     \
  /* Parsing-related errors */                                       \
  XX(INVALID_EOF_STATE, "stream ended at an unexpected time")        \
  XX(HEADER_OVERFLOW,                                                \
     "too many header bytes seen; overflow detected")                \
  XX(CLOSED_CONNECTION,                                              \
     "data received after completed connection: close message")      \
  XX(INVALID_VERSION, "invalid HTTP version")                        \
  XX(INVALID_STATUS, "invalid HTTP status code")                     \
  XX(INVALID_METHOD, "invalid HTTP method")                          \
  XX(INVALID_URL, "invalid URL")                                     \
  XX(INVALID_HOST, "invalid host")                                   \
  XX(INVALID_PORT, "invalid port")                                   \
  XX(INVALID_PATH, "invalid path")                                   \
  XX(INVALID_QUERY_STRING, "invalid query string")                   \
  XX(INVALID_FRAGMENT, "invalid fragment")                           \
  XX(LF_EXPECTED, "LF character expected")                           \
  XX(INVALID_HEADER_TOKEN, "invalid character in header")            \
  XX(INVALID_CONTENT_LENGTH,                                         \
     "invalid character in content-length header")                   \
  XX(UNEXPECTED_CONTENT_LENGTH,                                      \
     "unexpected content-length header")                             \
  XX(INVALID_CHUNK_SIZE,                                             \
     "invalid character in chunk size header")                       \
  XX(INVALID_CONSTANT, "invalid constant string")                    \
  XX(INVALID_INTERNAL_STATE, "encountered unexpected internal state")\
  XX(STRICT, "strict mode assertion failed")                         \
  XX(PAUSED, "parser is paused")                                     \
  XX(UNKNOWN, "an unknown error occurred")


    /* Define HNHTTP_HPE_* values for each errno value above */
#define HNHTTP_HTTP_ERRNO_GEN(n, s) HNHTTP_HPE_##n,
    enum http_errno {
        HNHTTP_HTTP_ERRNO_MAP(HNHTTP_HTTP_ERRNO_GEN)
    };
#undef HNHTTP_HTTP_ERRNO_GEN


    /* Get an http_errno value from an http_parser */
#define HNHTTP_HTTP_PARSER_ERRNO(p)            ((enum http_errno) (p)->http_errno)


#define HNHTTP_SET_ERRNO(e)                                                 \
do {                                                                 \
  parser->nread = nread;                                             \
  parser->http_errno = (e);                                          \
} while(0)

    /* Map errno values to strings for human-readable output */
#define HNHTTP_HTTP_STRERROR_GEN(n, s) { "HNHTTP_HPE_" #n, s },
    static struct {
        const char *name;
        const char *description;
    } http_strerror_tab[] = {
        HNHTTP_HTTP_ERRNO_MAP(HNHTTP_HTTP_STRERROR_GEN)
    };
#undef HNHTTP_HTTP_STRERROR_GEN





#define HNHTTP_START_STATE (parser->type == HTTP_REQUEST ? s_start_req : s_start_res)
#define HNHTTP_NEW_MESSAGE() (http_should_keep_alive(parser) ? HNHTTP_START_STATE : s_dead)
#define HNHTTP_CURRENT_STATE() p_state
#define HNHTTP_UPDATE_STATE(V) p_state = (enum state) (V);
#define HNHTTP_RETURN(V)                                                    \
do {                                                                 \
  parser->nread = nread;                                             \
  parser->state = HNHTTP_CURRENT_STATE();                                   \
  return (V);                                                        \
} while (0);






    /* Run the notify callback FOR, returning ER if it fails */
#define HNHTTP_CALLBACK_NOTIFY_(FOR, ER)                                    \
do {                                                                 \
  assert(HNHTTP_HTTP_PARSER_ERRNO(parser) == HNHTTP_HPE_OK);                       \
                                                                     \
  if (LIKELY(settings->on_##FOR)) {                                  \
    parser->state = HNHTTP_CURRENT_STATE();                                 \
    if (UNLIKELY(0 != settings->on_##FOR(parser))) {                 \
      HNHTTP_SET_ERRNO(HNHTTP_HPE_CB_##FOR);                                       \
        }                                                                \
    HNHTTP_UPDATE_STATE(parser->state);                                     \
                                                                     \
    /* We either errored above or got paused; get out */             \
    if (UNLIKELY(HNHTTP_HTTP_PARSER_ERRNO(parser) != HNHTTP_HPE_OK)) {             \
      return (ER);                                                   \
        }                                                                \
    }                                                                  \
} while (0)

    /* Run the notify callback FOR and consume the current byte */
#define HNHTTP_CALLBACK_NOTIFY(FOR)            HNHTTP_CALLBACK_NOTIFY_(FOR, p - data + 1)

    /* Run the notify callback FOR and don't consume the current byte */
#define HNHTTP_CALLBACK_NOTIFY_NOADVANCE(FOR)  HNHTTP_CALLBACK_NOTIFY_(FOR, p - data)

    /* Run data callback FOR with LEN bytes, returning ER if it fails */
#define HNHTTP_CALLBACK_DATA_(FOR, LEN, ER)                                 \
do {                                                                 \
  assert(HNHTTP_HTTP_PARSER_ERRNO(parser) == HNHTTP_HPE_OK);                       \
                                                                     \
  if (FOR##_mark) {                                                  \
    if (LIKELY(settings->on_##FOR)) {                                \
      parser->state = HNHTTP_CURRENT_STATE();                               \
      if (UNLIKELY(0 !=                                              \
                   settings->on_##FOR(parser, FOR##_mark, (LEN)))) { \
        HNHTTP_SET_ERRNO(HNHTTP_HPE_CB_##FOR);                                     \
            }                                                              \
      HNHTTP_UPDATE_STATE(parser->state);                                   \
                                                                     \
      /* We either errored above or got paused; get out */           \
      if (UNLIKELY(HNHTTP_HTTP_PARSER_ERRNO(parser) != HNHTTP_HPE_OK)) {           \
        return (ER);                                                 \
            }                                                              \
        }                                                                \
    FOR##_mark = NULL;                                               \
    }                                                                  \
} while (0)

    /* Run the data callback FOR and consume the current byte */
#define HNHTTP_CALLBACK_DATA(FOR)                                           \
    HNHTTP_CALLBACK_DATA_(FOR, p - FOR##_mark, p - data + 1)

    /* Run the data callback FOR and don't consume the current byte */
#define HNHTTP_CALLBACK_DATA_NOADVANCE(FOR)                                 \
    HNHTTP_CALLBACK_DATA_(FOR, p - FOR##_mark, p - data)










    /* Set the mark FOR; non-destructive if mark is already set */
#define HNHTTP_MARK(FOR)                                                    \
do {                                                                 \
  if (!FOR##_mark) {                                                 \
    FOR##_mark = p;                                                  \
    }                                                                  \
} while (0)

    /* Don't allow the total size of the HTTP headers (including the status
    * line) to exceed HN_MAXHTTPHEAD.  This check is here to protect
    * embedders against denial-of-service attacks where the attacker feeds
    * us a never-ending header that the embedder keeps buffering.
    *
    * This check is arguably the responsibility of embedders but we're doing
    * it on the embedder's behalf because most won't bother and this way we
    * make the web a little safer.  HN_MAXHTTPHEAD is still far bigger
    * than any reasonable request or response so this should never affect
    * day-to-day operation.
    */
#define HNHTTP_COUNT_HEADER_SIZE(V)                                         \
do {                                                                 \
  nread += (uint32_t)(V);                                            \
  if (UNLIKELY(nread > HN_MAXHTTPHEAD)) {                           \
    HNHTTP_SET_ERRNO(HNHTTP_HPE_HEADER_OVERFLOW);                                  \
    goto error;                                                      \
    }                                                                  \
} while (0)


#define HNHTTP_PARSING_HEADER(state) (state <= s_headers_done)

# define HNHTTP_STRICT_CHECK(cond)                                          \
do {                                                                 \
  if (cond) {                                                        \
    HNHTTP_SET_ERRNO(HNHTTP_HPE_STRICT);                                           \
    goto error;                                                      \
    }                                                                  \
} while (0)





    /* Our URL parser.
    *
    * This is designed to be shared by http_parser_execute() for URL validation,
    * hence it has a state transition + byte-for-byte interface. In addition, it
    * is meant to be embedded in http_parser_parse_url(), which does the dirty
    * work of turning state transitions URL components for its API.
    *
    * This function should only be invoked with non-space characters. It is
    * assumed that the caller cares about (and can detect) the transition between
    * URL and non-URL states by looking for these.
    */
    static enum state
        parse_url_char(enum state s, const char ch)
    {
        if (ch == ' ' || ch == '\r' || ch == '\n') {
            return s_dead;
        }


        if (ch == '\t' || ch == '\f') {
            return s_dead;
        }


        switch (s) {
        case s_req_spaces_before_url:
            /* Proxied requests are followed by scheme of an absolute URI (alpha).
            * All methods except CONNECT are followed by '/' or '*'.
            */

            if (ch == '/' || ch == '*') {
                return s_req_path;
            }

            if (IS_ALPHA(ch)) {
                return s_req_schema;
            }

            break;

        case s_req_schema:
            if (IS_ALPHA(ch)) {
                return s;
            }

            if (ch == ':') {
                return s_req_schema_slash;
            }

            break;

        case s_req_schema_slash:
            if (ch == '/') {
                return s_req_schema_slash_slash;
            }

            break;

        case s_req_schema_slash_slash:
            if (ch == '/') {
                return s_req_server_start;
            }

            break;

        case s_req_server_with_at:
            if (ch == '@') {
                return s_dead;
            }

            /* fall through */
        case s_req_server_start:
        case s_req_server:
            if (ch == '/') {
                return s_req_path;
            }

            if (ch == '?') {
                return s_req_query_string_start;
            }

            if (ch == '@') {
                return s_req_server_with_at;
            }

            if (IS_USERINFO_CHAR(ch) || ch == '[' || ch == ']') {
                return s_req_server;
            }

            break;

        case s_req_path:
            if (HN_IS_URL_CHAR(ch)) {
                return s;
            }

            switch (ch) {
            case '?':
                return s_req_query_string_start;

            case '#':
                return s_req_fragment_start;
            }

            break;

        case s_req_query_string_start:
        case s_req_query_string:
            if (HN_IS_URL_CHAR(ch)) {
                return s_req_query_string;
            }

            switch (ch) {
            case '?':
                /* allow extra '?' in query string */
                return s_req_query_string;

            case '#':
                return s_req_fragment_start;
            }

            break;

        case s_req_fragment_start:
            if (HN_IS_URL_CHAR(ch)) {
                return s_req_fragment;
            }

            switch (ch) {
            case '?':
                return s_req_fragment;

            case '#':
                return s;
            }

            break;

        case s_req_fragment:
            if (HN_IS_URL_CHAR(ch)) {
                return s;
            }

            switch (ch) {
            case '?':
            case '#':
                return s;
            }

            break;

        default:
            break;
        }

        /* We should never fall out of the switch above unless there's an error */
        return s_dead;
    }


    static enum http_host_state
        http_parse_host_char(enum http_host_state s, const char ch) {
        switch (s) {
        case s_http_userinfo:
        case s_http_userinfo_start:
            if (ch == '@') {
                return s_http_host_start;
            }

            if (IS_USERINFO_CHAR(ch)) {
                return s_http_userinfo;
            }
            break;

        case s_http_host_start:
            if (ch == '[') {
                return s_http_host_v6_start;
            }

            if (IS_HOST_CHAR(ch)) {
                return s_http_host;
            }

            break;

        case s_http_host:
            if (IS_HOST_CHAR(ch)) {
                return s_http_host;
            }

            /* fall through */
        case s_http_host_v6_end:
            if (ch == ':') {
                return s_http_host_port_start;
            }

            break;

        case s_http_host_v6:
            if (ch == ']') {
                return s_http_host_v6_end;
            }

            /* fall through */
        case s_http_host_v6_start:
            if (IS_HEX(ch) || ch == ':' || ch == '.') {
                return s_http_host_v6;
            }

            if (s == s_http_host_v6 && ch == '%') {
                return s_http_host_v6_zone_start;
            }
            break;

        case s_http_host_v6_zone:
            if (ch == ']') {
                return s_http_host_v6_end;
            }

            /* fall through */
        case s_http_host_v6_zone_start:
            /* RFC 6874 Zone ID consists of 1*( unreserved / pct-encoded) */
            if (IS_ALPHANUM(ch) || ch == '%' || ch == '.' || ch == '-' || ch == '_' ||
                ch == '~') {
                return s_http_host_v6_zone;
            }
            break;

        case s_http_host_port:
        case s_http_host_port_start:
            if (IS_NUM(ch)) {
                return s_http_host_port;
            }

            break;

        default:
            break;
        }
        return s_http_host_dead;
    }

    static int
        http_parse_host(const char * buf, http_parser_url *u, int found_at) {
        enum http_host_state s;

        const char *p;
        size_t buflen = u->field_data[UF_HOST].off + u->field_data[UF_HOST].len;

        assert(u->field_set & (1 << UF_HOST));

        u->field_data[UF_HOST].len = 0;

        s = found_at ? s_http_userinfo_start : s_http_host_start;

        for (p = buf + u->field_data[UF_HOST].off; p < buf + buflen; p++) {
            enum http_host_state new_s = http_parse_host_char(s, *p);

            if (new_s == s_http_host_dead) {
                return 1;
            }

            switch (new_s) {
            case s_http_host:
                if (s != s_http_host) {
                    u->field_data[UF_HOST].off = (uint16_t)(p - buf);
                }
                u->field_data[UF_HOST].len++;
                break;

            case s_http_host_v6:
                if (s != s_http_host_v6) {
                    u->field_data[UF_HOST].off = (uint16_t)(p - buf);
                }
                u->field_data[UF_HOST].len++;
                break;

            case s_http_host_v6_zone_start:
            case s_http_host_v6_zone:
                u->field_data[UF_HOST].len++;
                break;

            case s_http_host_port:
                if (s != s_http_host_port) {
                    u->field_data[UF_PORT].off = (uint16_t)(p - buf);
                    u->field_data[UF_PORT].len = 0;
                    u->field_set |= (1 << UF_PORT);
                }
                u->field_data[UF_PORT].len++;
                break;

            case s_http_userinfo:
                if (s != s_http_userinfo) {
                    u->field_data[UF_USERINFO].off = (uint16_t)(p - buf);
                    u->field_data[UF_USERINFO].len = 0;
                    u->field_set |= (1 << UF_USERINFO);
                }
                u->field_data[UF_USERINFO].len++;
                break;

            default:
                break;
            }
            s = new_s;
        }

        /* Make sure we don't end somewhere unexpected */
        switch (s) {
        case s_http_host_start:
        case s_http_host_v6_start:
        case s_http_host_v6:
        case s_http_host_v6_zone_start:
        case s_http_host_v6_zone:
        case s_http_host_port_start:
        case s_http_userinfo:
        case s_http_userinfo_start:
            return 1;
        default:
            break;
        }

        return 0;
    }


    static void
        http_parser_url_init(http_parser_url *u) {
        memset(u, 0, sizeof(*u));
    }

    static int
        http_parser_parse_url(const char *buf, size_t buflen, int is_connect,
        http_parser_url *u)
    {
        enum state s;
        const char *p;
        enum http_parser_url_fields uf, old_uf;
        int found_at = 0;

        if (buflen == 0) {
            return 1;
        }

        u->port = u->field_set = 0;
        s = is_connect ? s_req_server_start : s_req_spaces_before_url;
        old_uf = UF_MAX;

        for (p = buf; p < buf + buflen; p++) {
            s = parse_url_char(s, *p);

            /* Figure out the next field that we're operating on */
            switch (s) {
            case s_dead:
                return 1;

                /* Skip delimeters */
            case s_req_schema_slash:
            case s_req_schema_slash_slash:
            case s_req_server_start:
            case s_req_query_string_start:
            case s_req_fragment_start:
                continue;

            case s_req_schema:
                uf = UF_SCHEMA;
                break;

            case s_req_server_with_at:
                found_at = 1;

                /* fall through */
            case s_req_server:
                uf = UF_HOST;
                break;

            case s_req_path:
                uf = UF_PATH;
                break;

            case s_req_query_string:
                uf = UF_QUERY;
                break;

            case s_req_fragment:
                uf = UF_FRAGMENT;
                break;

            default:
                assert(!"Unexpected state");
                return 1;
            }

            /* Nothing's changed; soldier on */
            if (uf == old_uf) {
                u->field_data[uf].len++;
                continue;
            }

            u->field_data[uf].off = (uint16_t)(p - buf);
            u->field_data[uf].len = 1;

            u->field_set |= (1 << uf);
            old_uf = uf;
        }

        /* host must be present if there is a schema */
        /* parsing http:///toto will fail */
        if ((u->field_set & (1 << UF_SCHEMA)) &&
            (u->field_set & (1 << UF_HOST)) == 0) {
            return 1;
        }

        if (u->field_set & (1 << UF_HOST)) {
            if (http_parse_host(buf, u, found_at) != 0) {
                return 1;
            }
        }

        /* CONNECT requests can only contain "hostname:port" */
        if (is_connect && u->field_set != ((1 << UF_HOST) | (1 << UF_PORT))) {
            return 1;
        }

        if (u->field_set & (1 << UF_PORT)) {
            uint16_t off;
            uint16_t len;
            const char* p;
            const char* end;
            unsigned long v;

            off = u->field_data[UF_PORT].off;
            len = u->field_data[UF_PORT].len;
            end = buf + off + len;

            /* NOTE: The characters are already validated and are in the [0-9] range */
            assert((uint16_t)(off + len) <= buflen && "Port number overflow");
            v = 0;
            for (p = buf + off; p < end; p++) {
                v *= 10;
                v += *p - '0';

                /* Ports have a max value of 2^16 */
                if (v > 0xffff) {
                    return 1;
                }
            }

            u->port = (uint16_t)v;
        }

        return 0;
    }

    static void
        http_parser_init(http_parser *parser, enum http_parser_type t)
    {
        void *data = parser->data; /* preserve application data */
        memset(parser, 0, sizeof(*parser));
        parser->data = data;
        parser->type = t;
        parser->state = (t == HTTP_REQUEST ? s_start_req : (t == HTTP_RESPONSE ? s_start_res : s_start_req_or_res));
        parser->http_errno = HNHTTP_HPE_OK;
    }


    /* Initialize http_parser_settings members to 0
    */
    static void
        http_parser_settings_init(http_parser_settings *settings)
    {
        memset(settings, 0, sizeof(*settings));
    }


    /* Does the parser need to see an EOF to find the end of the message? */
    static int
        http_message_needs_eof(const http_parser *parser)
    {
        if (parser->type == HTTP_REQUEST) {
            return 0;
        }

        /* See RFC 2616 section 4.4 */
        if (parser->status_code / 100 == 1 || /* 1xx e.g. Continue */
            parser->status_code == 204 ||     /* No Content */
            parser->status_code == 304 ||     /* Not Modified */
            parser->flags & F_SKIPBODY) {     /* response to a HEAD request */
            return 0;
        }

        if ((parser->flags & F_CHUNKED) || parser->content_length != ULLONG_MAX) {
            return 0;
        }

        return 1;
    }

    /* If http_should_keep_alive() in the on_headers_complete or
    * on_message_complete callback returns 0, then this should be
    * the last message on the connection.
    * If you are the server, respond with the "Connection: close" header.
    * If you are the client, close the connection.
    */
    static int
        http_should_keep_alive(const http_parser *parser)
    {
        if (parser->http_major > 0 && parser->http_minor > 0) {
            /* HTTP/1.1 */
            if (parser->flags & F_CONNECTION_CLOSE) {
                return 0;
            }
        }
        else {
            /* HTTP/1.0 or earlier */
            if (!(parser->flags & F_CONNECTION_KEEP_ALIVE)) {
                return 0;
            }
        }

        return !http_message_needs_eof(parser);
    }


    /* Executes the parser. Returns number of parsed bytes. Sets
    * `parser->http_errno` on error. */


    static
        size_t http_parser_execute(http_parser *parser,
        const http_parser_settings *settings,
        const char *data,
        size_t len)
    {
        char c, ch;
        int8_t unhex_val;
        const char *p = data;
        const char *header_field_mark = 0;
        const char *header_value_mark = 0;
        const char *url_mark = 0;
        const char *body_mark = 0;
        const char *status_mark = 0;
        enum state p_state = (enum state) parser->state;
        const unsigned int lenient = parser->lenient_http_headers;
        uint32_t nread = parser->nread;

        /* We're in an error state. Don't bother doing anything. */
        if (HNHTTP_HTTP_PARSER_ERRNO(parser) != HNHTTP_HPE_OK) {
            return 0;
        }

        if (len == 0) {
            switch (HNHTTP_CURRENT_STATE()) {
            case s_body_identity_eof:
                /* Use of HNHTTP_CALLBACK_NOTIFY() here would erroneously return 1 byte read if
                * we got paused.
                */
                HNHTTP_CALLBACK_NOTIFY_NOADVANCE(message_complete);
                return 0;

            case s_dead:
            case s_start_req_or_res:
            case s_start_res:
            case s_start_req:
                return 0;

            default:
                HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_EOF_STATE);
                return 1;
            }
        }


        if (HNHTTP_CURRENT_STATE() == s_header_field)
            header_field_mark = data;
        if (HNHTTP_CURRENT_STATE() == s_header_value)
            header_value_mark = data;
        switch (HNHTTP_CURRENT_STATE()) {
        case s_req_path:
        case s_req_schema:
        case s_req_schema_slash:
        case s_req_schema_slash_slash:
        case s_req_server_start:
        case s_req_server:
        case s_req_server_with_at:
        case s_req_query_string_start:
        case s_req_query_string:
        case s_req_fragment_start:
        case s_req_fragment:
            url_mark = data;
            break;
        case s_res_status:
            status_mark = data;
            break;
        default:
            break;
        }

        for (p = data; p != data + len; p++) {
            ch = *p;

            if (HNHTTP_PARSING_HEADER(HNHTTP_CURRENT_STATE()))
                HNHTTP_COUNT_HEADER_SIZE(1);

        reexecute:
            switch (HNHTTP_CURRENT_STATE()) {

            case s_dead:
                /* this state is used after a 'Connection: close' message
                * the parser will error out if it reads another message
                */
                if (LIKELY(ch == HN_CR || ch == HN_LF))
                    break;

                HNHTTP_SET_ERRNO(HNHTTP_HPE_CLOSED_CONNECTION);
                goto error;

            case s_start_req_or_res:
            {
                if (ch == HN_CR || ch == HN_LF)
                    break;
                parser->flags = 0;
                parser->content_length = ULLONG_MAX;

                if (ch == 'H') {
                    HNHTTP_UPDATE_STATE(s_res_or_resp_H);

                    HNHTTP_CALLBACK_NOTIFY(message_begin);
                }
                else {
                    parser->type = HTTP_REQUEST;
                    HNHTTP_UPDATE_STATE(s_start_req);
                    goto reexecute;;
                }

                break;
            }

            case s_res_or_resp_H:
                if (ch == 'T') {
                    parser->type = HTTP_RESPONSE;
                    HNHTTP_UPDATE_STATE(s_res_HT);
                }
                else {
                    if (UNLIKELY(ch != 'E')) {
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONSTANT);
                        goto error;
                    }

                    parser->type = HTTP_REQUEST;
                    parser->method = HN_HTTP_HEAD;
                    parser->index = 2;
                    HNHTTP_UPDATE_STATE(s_req_method);
                }
                break;

            case s_start_res:
            {
                if (ch == HN_CR || ch == HN_LF)
                    break;
                parser->flags = 0;
                parser->content_length = ULLONG_MAX;

                if (ch == 'H') {
                    HNHTTP_UPDATE_STATE(s_res_H);
                }
                else {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONSTANT);
                    goto error;
                }

                HNHTTP_CALLBACK_NOTIFY(message_begin);
                break;
            }

            case s_res_H:
                HNHTTP_STRICT_CHECK(ch != 'T');
                HNHTTP_UPDATE_STATE(s_res_HT);
                break;

            case s_res_HT:
                HNHTTP_STRICT_CHECK(ch != 'T');
                HNHTTP_UPDATE_STATE(s_res_HTT);
                break;

            case s_res_HTT:
                HNHTTP_STRICT_CHECK(ch != 'P');
                HNHTTP_UPDATE_STATE(s_res_HTTP);
                break;

            case s_res_HTTP:
                HNHTTP_STRICT_CHECK(ch != '/');
                HNHTTP_UPDATE_STATE(s_res_http_major);
                break;

            case s_res_http_major:
                if (UNLIKELY(!IS_NUM(ch))) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_VERSION);
                    goto error;
                }

                parser->http_major = ch - '0';
                HNHTTP_UPDATE_STATE(s_res_http_dot);
                break;

            case s_res_http_dot:
            {
                if (UNLIKELY(ch != '.')) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_VERSION);
                    goto error;
                }

                HNHTTP_UPDATE_STATE(s_res_http_minor);
                break;
            }

            case s_res_http_minor:
                if (UNLIKELY(!IS_NUM(ch))) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_VERSION);
                    goto error;
                }

                parser->http_minor = ch - '0';
                HNHTTP_UPDATE_STATE(s_res_http_end);
                break;

            case s_res_http_end:
            {
                if (UNLIKELY(ch != ' ')) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_VERSION);
                    goto error;
                }

                HNHTTP_UPDATE_STATE(s_res_first_status_code);
                break;
            }

            case s_res_first_status_code:
            {
                if (!IS_NUM(ch)) {
                    if (ch == ' ') {
                        break;
                    }

                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_STATUS);
                    goto error;
                }
                parser->status_code = ch - '0';
                HNHTTP_UPDATE_STATE(s_res_status_code);
                break;
            }

            case s_res_status_code:
            {
                if (!IS_NUM(ch)) {
                    switch (ch) {
                    case ' ':
                        HNHTTP_UPDATE_STATE(s_res_status_start);
                        break;
                    case HN_CR:
                    case HN_LF:
                        HNHTTP_UPDATE_STATE(s_res_status_start);
                        goto reexecute;;
                        break;
                    default:
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_STATUS);
                        goto error;
                    }
                    break;
                }

                parser->status_code *= 10;
                parser->status_code += ch - '0';

                if (UNLIKELY(parser->status_code > 999)) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_STATUS);
                    goto error;
                }

                break;
            }

            case s_res_status_start:
            {
                HNHTTP_MARK(status);
                HNHTTP_UPDATE_STATE(s_res_status);
                parser->index = 0;

                if (ch == HN_CR || ch == HN_LF)
                    goto reexecute;;

                break;
            }

            case s_res_status:
                if (ch == HN_CR) {
                    HNHTTP_UPDATE_STATE(s_res_line_almost_done);
                    HNHTTP_CALLBACK_DATA(status);
                    break;
                }

                if (ch == HN_LF) {
                    HNHTTP_UPDATE_STATE(s_header_field_start);
                    HNHTTP_CALLBACK_DATA(status);
                    break;
                }

                break;

            case s_res_line_almost_done:
                HNHTTP_STRICT_CHECK(ch != HN_LF);
                HNHTTP_UPDATE_STATE(s_header_field_start);
                break;

            case s_start_req:
            {
                if (ch == HN_CR || ch == HN_LF)
                    break;
                parser->flags = 0;
                parser->content_length = ULLONG_MAX;

                if (UNLIKELY(!IS_ALPHA(ch))) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_METHOD);
                    goto error;
                }

                parser->method = 0;
                parser->index = 1;
                switch (ch) {
                case 'A': parser->method = HN_HTTP_ACL; break;
                case 'B': parser->method = HN_HTTP_BIND; break;
                case 'C': parser->method = HN_HTTP_CONNECT; /* or COPY, CHECKOUT */ break;
                case 'D': parser->method = HN_HTTP_DELETE; break;
                case 'G': parser->method = HN_HTTP_GET; break;
                case 'H': parser->method = HN_HTTP_HEAD; break;
                case 'L': parser->method = HN_HTTP_LOCK; /* or LINK */ break;
                case 'M': parser->method = HN_HTTP_MKCOL; /* or MOVE, MKACTIVITY, MERGE, M-SEARCH, MKCALENDAR */ break;
                case 'N': parser->method = HN_HTTP_NOTIFY; break;
                case 'O': parser->method = HN_HTTP_OPTIONS; break;
                case 'P': parser->method = HN_HTTP_POST;
                    /* or PROPFIND|PROPPATCH|PUT|PATCH|PURGE */
                    break;
                case 'R': parser->method = HN_HTTP_REPORT; /* or REBIND */ break;
                case 'S': parser->method = HN_HTTP_SUBSCRIBE; /* or SEARCH, SOURCE */ break;
                case 'T': parser->method = HN_HTTP_TRACE; break;
                case 'U': parser->method = HN_HTTP_UNLOCK; /* or UNSUBSCRIBE, UNBIND, UNLINK */ break;
                default:
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_METHOD);
                    goto error;
                }
                HNHTTP_UPDATE_STATE(s_req_method);

                HNHTTP_CALLBACK_NOTIFY(message_begin);

                break;
            }

            case s_req_method:
            {
                const char *matcher;
                if (UNLIKELY(ch == '\0')) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_METHOD);
                    goto error;
                }

                matcher = CHNStaticDeclare::obj().GetHttpMethodStr(parser->method).c_str();
                if (ch == ' ' && matcher[parser->index] == '\0') {
                    HNHTTP_UPDATE_STATE(s_req_spaces_before_url);
                }
                else if (ch == matcher[parser->index]) {
                    ; /* nada */
                }
                else if ((ch >= 'A' && ch <= 'Z') || ch == '-') {

                    switch (parser->method << 16 | parser->index << 8 | ch) {
#define XX(meth, pos, ch, new_meth) \
            case (HN_HTTP_##meth << 16 | pos << 8 | ch): \
              parser->method = HN_HTTP_##new_meth; break;

                        XX(POST, 1, 'U', PUT)
                            XX(POST, 1, 'A', PATCH)
                            XX(POST, 1, 'R', PROPFIND)
                            XX(PUT, 2, 'R', PURGE)
                            XX(CONNECT, 1, 'H', CHECKOUT)
                            XX(CONNECT, 2, 'P', COPY)
                            XX(MKCOL, 1, 'O', MOVE)
                            XX(MKCOL, 1, 'E', MERGE)
                            XX(MKCOL, 1, '-', MSEARCH)
                            XX(MKCOL, 2, 'A', MKACTIVITY)
                            XX(MKCOL, 3, 'A', MKCALENDAR)
                            XX(SUBSCRIBE, 1, 'E', SEARCH)
                            XX(SUBSCRIBE, 1, 'O', SOURCE)
                            XX(REPORT, 2, 'B', REBIND)
                            XX(PROPFIND, 4, 'P', PROPPATCH)
                            XX(LOCK, 1, 'I', LINK)
                            XX(UNLOCK, 2, 'S', UNSUBSCRIBE)
                            XX(UNLOCK, 2, 'B', UNBIND)
                            XX(UNLOCK, 3, 'I', UNLINK)
#undef XX
            default:
                HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_METHOD);
                goto error;
                    }
                }
                else {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_METHOD);
                    goto error;
                }

                ++parser->index;
                break;
            }

            case s_req_spaces_before_url:
            {
                if (ch == ' ') break;

                HNHTTP_MARK(url);
                if (parser->method == HN_HTTP_CONNECT) {
                    HNHTTP_UPDATE_STATE(s_req_server_start);
                }

                HNHTTP_UPDATE_STATE(parse_url_char(HNHTTP_CURRENT_STATE(), ch));
                if (UNLIKELY(HNHTTP_CURRENT_STATE() == s_dead)) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_URL);
                    goto error;
                }

                break;
            }

            case s_req_schema:
            case s_req_schema_slash:
            case s_req_schema_slash_slash:
            case s_req_server_start:
            {
                switch (ch) {
                    /* No whitespace allowed here */
                case ' ':
                case HN_CR:
                case HN_LF:
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_URL);
                    goto error;
                default:
                    HNHTTP_UPDATE_STATE(parse_url_char(HNHTTP_CURRENT_STATE(), ch));
                    if (UNLIKELY(HNHTTP_CURRENT_STATE() == s_dead)) {
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_URL);
                        goto error;
                    }
                }

                break;
            }

            case s_req_server:
            case s_req_server_with_at:
            case s_req_path:
            case s_req_query_string_start:
            case s_req_query_string:
            case s_req_fragment_start:
            case s_req_fragment:
            {
                switch (ch) {
                case ' ':
                    HNHTTP_UPDATE_STATE(s_req_http_start);
                    HNHTTP_CALLBACK_DATA(url);
                    break;
                case HN_CR:
                case HN_LF:
                    parser->http_major = 0;
                    parser->http_minor = 9;
                    HNHTTP_UPDATE_STATE((ch == HN_CR) ?
                    s_req_line_almost_done :
                                           s_header_field_start);
                    HNHTTP_CALLBACK_DATA(url);
                    break;
                default:
                    HNHTTP_UPDATE_STATE(parse_url_char(HNHTTP_CURRENT_STATE(), ch));
                    if (UNLIKELY(HNHTTP_CURRENT_STATE() == s_dead)) {
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_URL);
                        goto error;
                    }
                }
                break;
            }

            case s_req_http_start:
                switch (ch) {
                case ' ':
                    break;
                case 'H':
                    HNHTTP_UPDATE_STATE(s_req_http_H);
                    break;
                case 'I':
                    if (parser->method == HN_HTTP_SOURCE) {
                        HNHTTP_UPDATE_STATE(s_req_http_I);
                        break;
                    }
                    /* fall through */
                default:
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONSTANT);
                    goto error;
                }
                break;

            case s_req_http_H:
                HNHTTP_STRICT_CHECK(ch != 'T');
                HNHTTP_UPDATE_STATE(s_req_http_HT);
                break;

            case s_req_http_HT:
                HNHTTP_STRICT_CHECK(ch != 'T');
                HNHTTP_UPDATE_STATE(s_req_http_HTT);
                break;

            case s_req_http_HTT:
                HNHTTP_STRICT_CHECK(ch != 'P');
                HNHTTP_UPDATE_STATE(s_req_http_HTTP);
                break;

            case s_req_http_I:
                HNHTTP_STRICT_CHECK(ch != 'C');
                HNHTTP_UPDATE_STATE(s_req_http_IC);
                break;

            case s_req_http_IC:
                HNHTTP_STRICT_CHECK(ch != 'E');
                HNHTTP_UPDATE_STATE(s_req_http_HTTP);  /* Treat "ICE" as "HTTP". */
                break;

            case s_req_http_HTTP:
                HNHTTP_STRICT_CHECK(ch != '/');
                HNHTTP_UPDATE_STATE(s_req_http_major);
                break;

            case s_req_http_major:
                if (UNLIKELY(!IS_NUM(ch))) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_VERSION);
                    goto error;
                }

                parser->http_major = ch - '0';
                HNHTTP_UPDATE_STATE(s_req_http_dot);
                break;

            case s_req_http_dot:
            {
                if (UNLIKELY(ch != '.')) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_VERSION);
                    goto error;
                }

                HNHTTP_UPDATE_STATE(s_req_http_minor);
                break;
            }

            case s_req_http_minor:
                if (UNLIKELY(!IS_NUM(ch))) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_VERSION);
                    goto error;
                }

                parser->http_minor = ch - '0';
                HNHTTP_UPDATE_STATE(s_req_http_end);
                break;

            case s_req_http_end:
            {
                if (ch == HN_CR) {
                    HNHTTP_UPDATE_STATE(s_req_line_almost_done);
                    break;
                }

                if (ch == HN_LF) {
                    HNHTTP_UPDATE_STATE(s_header_field_start);
                    break;
                }

                HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_VERSION);
                goto error;
                break;
            }

            /* end of request line */
            case s_req_line_almost_done:
            {
                if (UNLIKELY(ch != HN_LF)) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_LF_EXPECTED);
                    goto error;
                }

                HNHTTP_UPDATE_STATE(s_header_field_start);
                break;
            }

            case s_header_field_start:
            {
                if (ch == HN_CR) {
                    HNHTTP_UPDATE_STATE(s_headers_almost_done);
                    break;
                }

                if (ch == HN_LF) {
                    /* they might be just sending \n instead of \r\n so this would be
                    * the second \n to denote the end of headers*/
                    HNHTTP_UPDATE_STATE(s_headers_almost_done);
                    goto reexecute;;
                }

                c = HN_STRICT_TOKEN(ch);

                if (UNLIKELY(!c)) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_HEADER_TOKEN);
                    goto error;
                }

                HNHTTP_MARK(header_field);

                parser->index = 0;
                HNHTTP_UPDATE_STATE(s_header_field);

                switch (c) {
                case 'c':
                    parser->header_state = h_C;
                    break;

                case 'p':
                    parser->header_state = h_matching_proxy_connection;
                    break;

                case 't':
                    parser->header_state = h_matching_transfer_encoding;
                    break;

                case 'u':
                    parser->header_state = h_matching_upgrade;
                    break;

                default:
                    parser->header_state = h_general;
                    break;
                }
                break;
            }

            case s_header_field:
            {
                const char* start = p;
                for (; p != data + len; p++) {
                    ch = *p;
                    c = HN_STRICT_TOKEN(ch);

                    if (!c)
                        break;

                    switch (parser->header_state) {
                    case h_general: {
                        size_t left = data + len - p;
                        const char* pe = p + HN_MIN(left, HN_MAXHTTPHEAD);
                        while (p + 1 < pe && HN_STRICT_TOKEN(p[1])) {
                            p++;
                        }
                        break;
                    }

                    case h_C:
                        parser->index++;
                        parser->header_state = (c == 'o' ? h_CO : h_general);
                        break;

                    case h_CO:
                        parser->index++;
                        parser->header_state = (c == 'n' ? h_CON : h_general);
                        break;

                    case h_CON:
                        parser->index++;
                        switch (c) {
                        case 'n':
                            parser->header_state = h_matching_connection;
                            break;
                        case 't':
                            parser->header_state = h_matching_content_length;
                            break;
                        default:
                            parser->header_state = h_general;
                            break;
                        }
                        break;

                        /* connection */

                    case h_matching_connection:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_CONNECTION) - 1
                            || c != HNHTTP_CONNECTION[parser->index]) {
                            parser->header_state = h_general;
                        }
                        else if (parser->index == sizeof(HNHTTP_CONNECTION) - 2) {
                            parser->header_state = h_connection;
                        }
                        break;

                        /* proxy-connection */

                    case h_matching_proxy_connection:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_PROXY_CONNECTION) - 1
                            || c != HNHTTP_PROXY_CONNECTION[parser->index]) {
                            parser->header_state = h_general;
                        }
                        else if (parser->index == sizeof(HNHTTP_PROXY_CONNECTION) - 2) {
                            parser->header_state = h_connection;
                        }
                        break;

                        /* content-length */

                    case h_matching_content_length:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_CONTENT_LENGTH) - 1
                            || c != HNHTTP_CONTENT_LENGTH[parser->index]) {
                            parser->header_state = h_general;
                        }
                        else if (parser->index == sizeof(HNHTTP_CONTENT_LENGTH) - 2) {
                            parser->header_state = h_content_length;
                        }
                        break;

                        /* transfer-encoding */

                    case h_matching_transfer_encoding:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_TRANSFER_ENCODING) - 1
                            || c != HNHTTP_TRANSFER_ENCODING[parser->index]) {
                            parser->header_state = h_general;
                        }
                        else if (parser->index == sizeof(HNHTTP_TRANSFER_ENCODING) - 2) {
                            parser->header_state = h_transfer_encoding;
                        }
                        break;

                        /* upgrade */

                    case h_matching_upgrade:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_UPGRADE) - 1
                            || c != HNHTTP_UPGRADE[parser->index]) {
                            parser->header_state = h_general;
                        }
                        else if (parser->index == sizeof(HNHTTP_UPGRADE) - 2) {
                            parser->header_state = h_upgrade;
                        }
                        break;

                    case h_connection:
                    case h_content_length:
                    case h_transfer_encoding:
                    case h_upgrade:
                        if (ch != ' ') parser->header_state = h_general;
                        break;

                    default:
                        assert(0 && "Unknown header_state");
                        break;
                    }
                }

                if (p == data + len) {
                    --p;
                    HNHTTP_COUNT_HEADER_SIZE(p - start);
                    break;
                }

                HNHTTP_COUNT_HEADER_SIZE(p - start);

                if (ch == ':') {
                    HNHTTP_UPDATE_STATE(s_header_value_discard_ws);
                    HNHTTP_CALLBACK_DATA(header_field);
                    break;
                }

                HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_HEADER_TOKEN);
                goto error;
            }

            case s_header_value_discard_ws:
                if (ch == ' ' || ch == '\t') break;

                if (ch == HN_CR) {
                    HNHTTP_UPDATE_STATE(s_header_value_discard_ws_almost_done);
                    break;
                }

                if (ch == HN_LF) {
                    HNHTTP_UPDATE_STATE(s_header_value_discard_lws);
                    break;
                }

                /* fall through */

            case s_header_value_start:
            {
                HNHTTP_MARK(header_value);

                HNHTTP_UPDATE_STATE(s_header_value);
                parser->index = 0;

                c = LOWER(ch);

                switch (parser->header_state) {
                case h_upgrade:
                    parser->flags |= F_UPGRADE;
                    parser->header_state = h_general;
                    break;

                case h_transfer_encoding:
                    /* looking for 'Transfer-Encoding: chunked' */
                    if ('c' == c) {
                        parser->header_state = h_matching_transfer_encoding_chunked;
                    }
                    else {
                        parser->header_state = h_general;
                    }
                    break;

                case h_content_length:
                    if (UNLIKELY(!IS_NUM(ch))) {
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONTENT_LENGTH);
                        goto error;
                    }

                    if (parser->flags & F_CONTENTLENGTH) {
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_UNEXPECTED_CONTENT_LENGTH);
                        goto error;
                    }

                    parser->flags |= F_CONTENTLENGTH;
                    parser->content_length = ch - '0';
                    parser->header_state = h_content_length_num;
                    break;

                    /* when obsolete line folding is encountered for content length
                    * continue to the s_header_value state */
                case h_content_length_ws:
                    break;

                case h_connection:
                    /* looking for 'Connection: keep-alive' */
                    if (c == 'k') {
                        parser->header_state = h_matching_connection_keep_alive;
                        /* looking for 'Connection: close' */
                    }
                    else if (c == 'c') {
                        parser->header_state = h_matching_connection_close;
                    }
                    else if (c == 'u') {
                        parser->header_state = h_matching_connection_upgrade;
                    }
                    else {
                        parser->header_state = h_matching_connection_token;
                    }
                    break;

                    /* Multi-value `Connection` header */
                case h_matching_connection_token_start:
                    break;

                default:
                    parser->header_state = h_general;
                    break;
                }
                break;
            }

            case s_header_value:
            {
                const char* start = p;
                enum header_states h_state = (enum header_states) parser->header_state;
                for (; p != data + len; p++) {
                    ch = *p;
                    if (ch == HN_CR) {
                        HNHTTP_UPDATE_STATE(s_header_almost_done);
                        parser->header_state = h_state;
                        HNHTTP_CALLBACK_DATA(header_value);
                        break;
                    }

                    if (ch == HN_LF) {
                        HNHTTP_UPDATE_STATE(s_header_almost_done);
                        HNHTTP_COUNT_HEADER_SIZE(p - start);
                        parser->header_state = h_state;
                        HNHTTP_CALLBACK_DATA_NOADVANCE(header_value);
                        goto reexecute;;
                    }

                    if (!lenient && !HN_IS_HEADER_CHAR(ch)) {
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_HEADER_TOKEN);
                        goto error;
                    }

                    c = LOWER(ch);

                    switch (h_state) {
                    case h_general:
                    {
                        size_t left = data + len - p;
                        const char* pe = p + HN_MIN(left, HN_MAXHTTPHEAD);

                        for (; p != pe; p++) {
                            ch = *p;
                            if (ch == HN_CR || ch == HN_LF) {
                                --p;
                                break;
                            }
                            if (!lenient && !HN_IS_HEADER_CHAR(ch)) {
                                HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_HEADER_TOKEN);
                                goto error;
                            }
                        }
                        if (p == data + len)
                            --p;
                        break;
                    }

                    case h_connection:
                    case h_transfer_encoding:
                        assert(0 && "Shouldn't get here.");
                        break;

                    case h_content_length:
                        if (ch == ' ') break;
                        h_state = h_content_length_num;
                        /* fall through */

                    case h_content_length_num:
                    {
                        uint64_t t;

                        if (ch == ' ') {
                            h_state = h_content_length_ws;
                            break;
                        }

                        if (UNLIKELY(!IS_NUM(ch))) {
                            HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONTENT_LENGTH);
                            parser->header_state = h_state;
                            goto error;
                        }

                        t = parser->content_length;
                        t *= 10;
                        t += ch - '0';

                        /* Overflow? Test against a conservative limit for simplicity. */
                        if (UNLIKELY((ULLONG_MAX - 10) / 10 < parser->content_length)) {
                            HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONTENT_LENGTH);
                            parser->header_state = h_state;
                            goto error;
                        }

                        parser->content_length = t;
                        break;
                    }

                    case h_content_length_ws:
                        if (ch == ' ') break;
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONTENT_LENGTH);
                        parser->header_state = h_state;
                        goto error;

                        /* Transfer-Encoding: chunked */
                    case h_matching_transfer_encoding_chunked:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_CHUNKED) - 1
                            || c != HNHTTP_CHUNKED[parser->index]) {
                            h_state = h_general;
                        }
                        else if (parser->index == sizeof(HNHTTP_CHUNKED) - 2) {
                            h_state = h_transfer_encoding_chunked;
                        }
                        break;

                    case h_matching_connection_token_start:
                        /* looking for 'Connection: keep-alive' */
                        if (c == 'k') {
                            h_state = h_matching_connection_keep_alive;
                            /* looking for 'Connection: close' */
                        }
                        else if (c == 'c') {
                            h_state = h_matching_connection_close;
                        }
                        else if (c == 'u') {
                            h_state = h_matching_connection_upgrade;
                        }
                        else if (HN_STRICT_TOKEN(c)) {
                            h_state = h_matching_connection_token;
                        }
                        else if (c == ' ' || c == '\t') {
                            /* Skip lws */
                        }
                        else {
                            h_state = h_general;
                        }
                        break;

                        /* looking for 'Connection: keep-alive' */
                    case h_matching_connection_keep_alive:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_KEEP_ALIVE) - 1
                            || c != HNHTTP_KEEP_ALIVE[parser->index]) {
                            h_state = h_matching_connection_token;
                        }
                        else if (parser->index == sizeof(HNHTTP_KEEP_ALIVE) - 2) {
                            h_state = h_connection_keep_alive;
                        }
                        break;

                        /* looking for 'Connection: close' */
                    case h_matching_connection_close:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_CLOSE) - 1 || c != HNHTTP_CLOSE[parser->index]) {
                            h_state = h_matching_connection_token;
                        }
                        else if (parser->index == sizeof(HNHTTP_CLOSE) - 2) {
                            h_state = h_connection_close;
                        }
                        break;

                        /* looking for 'Connection: upgrade' */
                    case h_matching_connection_upgrade:
                        parser->index++;
                        if (parser->index > sizeof(HNHTTP_UPGRADE) - 1 ||
                            c != HNHTTP_UPGRADE[parser->index]) {
                            h_state = h_matching_connection_token;
                        }
                        else if (parser->index == sizeof(HNHTTP_UPGRADE) - 2) {
                            h_state = h_connection_upgrade;
                        }
                        break;

                    case h_matching_connection_token:
                        if (ch == ',') {
                            h_state = h_matching_connection_token_start;
                            parser->index = 0;
                        }
                        break;

                    case h_transfer_encoding_chunked:
                        if (ch != ' ') h_state = h_general;
                        break;

                    case h_connection_keep_alive:
                    case h_connection_close:
                    case h_connection_upgrade:
                        if (ch == ',') {
                            if (h_state == h_connection_keep_alive) {
                                parser->flags |= F_CONNECTION_KEEP_ALIVE;
                            }
                            else if (h_state == h_connection_close) {
                                parser->flags |= F_CONNECTION_CLOSE;
                            }
                            else if (h_state == h_connection_upgrade) {
                                parser->flags |= F_CONNECTION_UPGRADE;
                            }
                            h_state = h_matching_connection_token_start;
                            parser->index = 0;
                        }
                        else if (ch != ' ') {
                            h_state = h_matching_connection_token;
                        }
                        break;

                    default:
                        HNHTTP_UPDATE_STATE(s_header_value);
                        h_state = h_general;
                        break;
                    }
                }
                parser->header_state = h_state;

                if (p == data + len)
                    --p;

                HNHTTP_COUNT_HEADER_SIZE(p - start);
                break;
            }

            case s_header_almost_done:
            {
                if (UNLIKELY(ch != HN_LF)) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_LF_EXPECTED);
                    goto error;
                }

                HNHTTP_UPDATE_STATE(s_header_value_lws);
                break;
            }

            case s_header_value_lws:
            {
                if (ch == ' ' || ch == '\t') {
                    if (parser->header_state == h_content_length_num) {
                        /* treat obsolete line folding as space */
                        parser->header_state = h_content_length_ws;
                    }
                    HNHTTP_UPDATE_STATE(s_header_value_start);
                    goto reexecute;;
                }

                /* finished the header */
                switch (parser->header_state) {
                case h_connection_keep_alive:
                    parser->flags |= F_CONNECTION_KEEP_ALIVE;
                    break;
                case h_connection_close:
                    parser->flags |= F_CONNECTION_CLOSE;
                    break;
                case h_transfer_encoding_chunked:
                    parser->flags |= F_CHUNKED;
                    break;
                case h_connection_upgrade:
                    parser->flags |= F_CONNECTION_UPGRADE;
                    break;
                default:
                    break;
                }

                HNHTTP_UPDATE_STATE(s_header_field_start);
                goto reexecute;;
            }

            case s_header_value_discard_ws_almost_done:
            {
                HNHTTP_STRICT_CHECK(ch != HN_LF);
                HNHTTP_UPDATE_STATE(s_header_value_discard_lws);
                break;
            }

            case s_header_value_discard_lws:
            {
                if (ch == ' ' || ch == '\t') {
                    HNHTTP_UPDATE_STATE(s_header_value_discard_ws);
                    break;
                }
                else {
                    switch (parser->header_state) {
                    case h_connection_keep_alive:
                        parser->flags |= F_CONNECTION_KEEP_ALIVE;
                        break;
                    case h_connection_close:
                        parser->flags |= F_CONNECTION_CLOSE;
                        break;
                    case h_connection_upgrade:
                        parser->flags |= F_CONNECTION_UPGRADE;
                        break;
                    case h_transfer_encoding_chunked:
                        parser->flags |= F_CHUNKED;
                        break;
                    case h_content_length:
                        /* do not allow empty content length */
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONTENT_LENGTH);
                        goto error;
                        break;
                    default:
                        break;
                    }

                    /* header value was empty */
                    HNHTTP_MARK(header_value);
                    HNHTTP_UPDATE_STATE(s_header_field_start);
                    HNHTTP_CALLBACK_DATA_NOADVANCE(header_value);
                    goto reexecute;;
                }
            }

            case s_headers_almost_done:
            {
                HNHTTP_STRICT_CHECK(ch != HN_LF);

                if (parser->flags & F_TRAILING) {
                    /* End of a chunked request */
                    HNHTTP_UPDATE_STATE(s_message_done);
                    HNHTTP_CALLBACK_NOTIFY_NOADVANCE(chunk_complete);
                    goto reexecute;;
                }

                /* Cannot use chunked encoding and a content-length header together
                per the HTTP specification. */
                if ((parser->flags & F_CHUNKED) &&
                    (parser->flags & F_CONTENTLENGTH)) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_UNEXPECTED_CONTENT_LENGTH);
                    goto error;
                }

                HNHTTP_UPDATE_STATE(s_headers_done);

                /* Set this here so that on_headers_complete() callbacks can see it */
                if ((parser->flags & F_UPGRADE) &&
                    (parser->flags & F_CONNECTION_UPGRADE)) {
                    /* For responses, "Upgrade: foo" and "Connection: upgrade" are
                    * mandatory only when it is a 101 Switching Protocols response,
                    * otherwise it is purely informational, to announce support.
                    */
                    parser->upgrade =
                        (parser->type == HTTP_REQUEST || parser->status_code == 101);
                }
                else {
                    parser->upgrade = (parser->method == HN_HTTP_CONNECT);
                }

                /* Here we call the headers_complete callback. This is somewhat
                * different than other callbacks because if the user returns 1, we
                * will interpret that as saying that this message has no body. This
                * is needed for the annoying case of recieving a response to a HEAD
                * request.
                *
                * We'd like to use HNHTTP_CALLBACK_NOTIFY_NOADVANCE() here but we cannot, so
                * we have to simulate it by handling a change in errno below.
                */
                if (settings->on_headers_complete) {
                    switch (settings->on_headers_complete(parser)) {
                    case 0:
                        break;

                    case 2:
                        parser->upgrade = 1;

                        /* fall through */
                    case 1:
                        parser->flags |= F_SKIPBODY;
                        break;

                    default:
                        HNHTTP_SET_ERRNO(HNHTTP_HPE_CB_headers_complete);
                        HNHTTP_RETURN(p - data); /* Error */
                    }
                }

                if (HNHTTP_HTTP_PARSER_ERRNO(parser) != HNHTTP_HPE_OK) {
                    HNHTTP_RETURN(p - data);
                }

                goto reexecute;;
            }

            case s_headers_done:
            {
                int hasBody;
                HNHTTP_STRICT_CHECK(ch != HN_LF);

                parser->nread = 0;
                nread = 0;

                hasBody = parser->flags & F_CHUNKED ||
                    (parser->content_length > 0 && parser->content_length != ULLONG_MAX);
                if (parser->upgrade && (parser->method == HN_HTTP_CONNECT ||
                    (parser->flags & F_SKIPBODY) || !hasBody)) {
                    /* Exit, the rest of the message is in a different protocol. */
                    HNHTTP_UPDATE_STATE(HNHTTP_NEW_MESSAGE());
                    HNHTTP_CALLBACK_NOTIFY(message_complete);
                    HNHTTP_RETURN((p - data) + 1);
                }

                if (parser->flags & F_SKIPBODY) {
                    HNHTTP_UPDATE_STATE(HNHTTP_NEW_MESSAGE());
                    HNHTTP_CALLBACK_NOTIFY(message_complete);
                }
                else if (parser->flags & F_CHUNKED) {
                    /* chunked encoding - ignore Content-Length header */
                    HNHTTP_UPDATE_STATE(s_chunk_size_start);
                }
                else {
                    if (parser->content_length == 0) {
                        /* Content-Length header given but zero: Content-Length: 0\r\n */
                        HNHTTP_UPDATE_STATE(HNHTTP_NEW_MESSAGE());
                        HNHTTP_CALLBACK_NOTIFY(message_complete);
                    }
                    else if (parser->content_length != ULLONG_MAX) {
                        /* Content-Length header given and non-zero */
                        HNHTTP_UPDATE_STATE(s_body_identity);
                    }
                    else {
                        if (!http_message_needs_eof(parser)) {
                            /* Assume content-length 0 - read the next */
                            HNHTTP_UPDATE_STATE(HNHTTP_NEW_MESSAGE());
                            HNHTTP_CALLBACK_NOTIFY(message_complete);
                        }
                        else {
                            /* Read body until EOF */
                            HNHTTP_UPDATE_STATE(s_body_identity_eof);
                        }
                    }
                }

                break;
            }

            case s_body_identity:
            {
                uint64_t to_read = HN_MIN(parser->content_length,
                    (uint64_t)((data + len) - p));

                assert(parser->content_length != 0
                    && parser->content_length != ULLONG_MAX);

                /* The difference between advancing content_length and p is because
                * the latter will automaticaly advance on the next loop iteration.
                * Further, if content_length ends up at 0, we want to see the last
                * byte again for our message complete callback.
                */
                HNHTTP_MARK(body);
                parser->content_length -= to_read;
                p += to_read - 1;

                if (parser->content_length == 0) {
                    HNHTTP_UPDATE_STATE(s_message_done);

                    /* Mimic HNHTTP_CALLBACK_DATA_NOADVANCE() but with one extra byte.
                    *
                    * The alternative to doing this is to wait for the next byte to
                    * trigger the data callback, just as in every other case. The
                    * problem with this is that this makes it difficult for the test
                    * harness to distinguish between complete-on-EOF and
                    * complete-on-length. It's not clear that this distinction is
                    * important for applications, but let's keep it for now.
                    */
                    HNHTTP_CALLBACK_DATA_(body, p - body_mark + 1, p - data);
                    goto reexecute;;
                }

                break;
            }

            /* read until EOF */
            case s_body_identity_eof:
                HNHTTP_MARK(body);
                p = data + len - 1;

                break;

            case s_message_done:
                HNHTTP_UPDATE_STATE(HNHTTP_NEW_MESSAGE());
                HNHTTP_CALLBACK_NOTIFY(message_complete);
                if (parser->upgrade) {
                    /* Exit, the rest of the message is in a different protocol. */
                    HNHTTP_RETURN((p - data) + 1);
                }
                break;

            case s_chunk_size_start:
            {
                assert(nread == 1);
                assert(parser->flags & F_CHUNKED);

                unhex_val = HN_NUMBER_UNHEX[(unsigned char)ch];
                if (UNLIKELY(unhex_val == -1)) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CHUNK_SIZE);
                    goto error;
                }

                parser->content_length = unhex_val;
                HNHTTP_UPDATE_STATE(s_chunk_size);
                break;
            }

            case s_chunk_size:
            {
                uint64_t t;

                assert(parser->flags & F_CHUNKED);

                if (ch == HN_CR) {
                    HNHTTP_UPDATE_STATE(s_chunk_size_almost_done);
                    break;
                }

                unhex_val = HN_NUMBER_UNHEX[(unsigned char)ch];

                if (unhex_val == -1) {
                    if (ch == ';' || ch == ' ') {
                        HNHTTP_UPDATE_STATE(s_chunk_parameters);
                        break;
                    }

                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CHUNK_SIZE);
                    goto error;
                }

                t = parser->content_length;
                t *= 16;
                t += unhex_val;

                /* Overflow? Test against a conservative limit for simplicity. */
                if (UNLIKELY((ULLONG_MAX - 16) / 16 < parser->content_length)) {
                    HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_CONTENT_LENGTH);
                    goto error;
                }

                parser->content_length = t;
                break;
            }

            case s_chunk_parameters:
            {
                assert(parser->flags & F_CHUNKED);
                /* just ignore this shit. TODO check for overflow */
                if (ch == HN_CR) {
                    HNHTTP_UPDATE_STATE(s_chunk_size_almost_done);
                    break;
                }
                break;
            }

            case s_chunk_size_almost_done:
            {
                assert(parser->flags & F_CHUNKED);
                HNHTTP_STRICT_CHECK(ch != HN_LF);

                parser->nread = 0;
                nread = 0;

                if (parser->content_length == 0) {
                    parser->flags |= F_TRAILING;
                    HNHTTP_UPDATE_STATE(s_header_field_start);
                }
                else {
                    HNHTTP_UPDATE_STATE(s_chunk_data);
                }
                HNHTTP_CALLBACK_NOTIFY(chunk_header);
                break;
            }

            case s_chunk_data:
            {
                uint64_t to_read = HN_MIN(parser->content_length,
                    (uint64_t)((data + len) - p));

                assert(parser->flags & F_CHUNKED);
                assert(parser->content_length != 0
                    && parser->content_length != ULLONG_MAX);

                /* See the explanation in s_body_identity for why the content
                * length and data pointers are managed this way.
                */
                HNHTTP_MARK(body);
                parser->content_length -= to_read;
                p += to_read - 1;

                if (parser->content_length == 0) {
                    HNHTTP_UPDATE_STATE(s_chunk_data_almost_done);
                }

                break;
            }

            case s_chunk_data_almost_done:
                assert(parser->flags & F_CHUNKED);
                assert(parser->content_length == 0);
                HNHTTP_STRICT_CHECK(ch != HN_CR);
                HNHTTP_UPDATE_STATE(s_chunk_data_done);
                HNHTTP_CALLBACK_DATA(body);
                break;

            case s_chunk_data_done:
                assert(parser->flags & F_CHUNKED);
                HNHTTP_STRICT_CHECK(ch != HN_LF);
                parser->nread = 0;
                nread = 0;
                HNHTTP_UPDATE_STATE(s_chunk_size_start);
                HNHTTP_CALLBACK_NOTIFY(chunk_complete);
                break;

            default:
                assert(0 && "unhandled state");
                HNHTTP_SET_ERRNO(HNHTTP_HPE_INVALID_INTERNAL_STATE);
                goto error;
            }
        }

        /* Run callbacks for any marks that we have leftover after we ran out of
        * bytes. There should be at most one of these set, so it's OK to invoke
        * them in series (unset marks will not result in callbacks).
        *
        * We use the NOADVANCE() variety of callbacks here because 'p' has already
        * overflowed 'data' and this allows us to correct for the off-by-one that
        * we'd otherwise have (since HNHTTP_CALLBACK_DATA() is meant to be run with a 'p'
        * value that's in-bounds).
        */

        assert(((header_field_mark ? 1 : 0) +
            (header_value_mark ? 1 : 0) +
            (url_mark ? 1 : 0) +
            (body_mark ? 1 : 0) +
            (status_mark ? 1 : 0)) <= 1);

        HNHTTP_CALLBACK_DATA_NOADVANCE(header_field);
        HNHTTP_CALLBACK_DATA_NOADVANCE(header_value);
        HNHTTP_CALLBACK_DATA_NOADVANCE(url);
        HNHTTP_CALLBACK_DATA_NOADVANCE(body);
        HNHTTP_CALLBACK_DATA_NOADVANCE(status);

        HNHTTP_RETURN(len);

    error:
        if (HNHTTP_HTTP_PARSER_ERRNO(parser) == HNHTTP_HPE_OK) {
            HNHTTP_SET_ERRNO(HNHTTP_HPE_UNKNOWN);
        }

        HNHTTP_RETURN(p - data);
    }



    /* Return a string name of the given error */

    static const char *
        http_errno_name(enum http_errno err) {
        assert(((size_t)err) < ARRAY_SIZE(http_strerror_tab));
        return http_strerror_tab[err].name;
    }

    /* Return a string description of the given error */
    static const char *
        http_errno_description(enum http_errno err) {
        assert(((size_t)err) < ARRAY_SIZE(http_strerror_tab));
        return http_strerror_tab[err].description;
    }


    /* Pause or un-pause the parser; a nonzero value pauses */
    static void
        http_parser_pause(http_parser *parser, int paused) {
        /* Users should only be pausing/unpausing a parser that is not in an error
        * state. In non-debug builds, there's not much that we can do about this
        * other than ignore it.
        */
        if (HNHTTP_HTTP_PARSER_ERRNO(parser) == HNHTTP_HPE_OK ||
            HNHTTP_HTTP_PARSER_ERRNO(parser) == HNHTTP_HPE_PAUSED) {
            uint32_t nread = parser->nread; /* used by the HNHTTP_SET_ERRNO macro */
            HNHTTP_SET_ERRNO((paused) ? HNHTTP_HPE_PAUSED : HNHTTP_HPE_OK);
        }
        else {
            assert(0 && "Attempting to pause parser in error state");
        }
    }


    /* Checks if this is the final chunk of the body. */
    static int
        http_body_is_final(const http_parser *parser) {
        return parser->state == s_message_done;
    }


#undef HTTP_ERRNO_MAP
#undef HNHTTP_HTTP_PARSER_ERRNO
#undef HNHTTP_SET_ERRNO
#undef HNHTTP_START_STATE
#undef HNHTTP_NEW_MESSAGE
#undef HNHTTP_CURRENT_STATE
#undef HNHTTP_UPDATE_STATE
#undef HNHTTP_RETURN
#undef HNHTTP_CALLBACK_NOTIFY_
#undef HNHTTP_CALLBACK_NOTIFY
#undef HNHTTP_CALLBACK_NOTIFY_NOADVANCE
#undef HNHTTP_CALLBACK_DATA_
#undef HNHTTP_CALLBACK_DATA
#undef HNHTTP_CALLBACK_DATA_NOADVANCE
#undef HNHTTP_MARK
#undef HNHTTP_COUNT_HEADER_SIZE
#undef HNHTTP_PARSING_HEADER
#undef HNHTTP_STRICT_CHECK
























}    /*hnhttpparser*/


#endif