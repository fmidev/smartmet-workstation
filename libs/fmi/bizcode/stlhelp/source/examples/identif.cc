// k2/identify/identif.cc
#include"identif.h"
#include<cctype>

istream& operator>>(istream& is, Identifier& N)
{
    if(!is.ipfx()) return is;

    /* The prefixed input  function ipfx() ('p' for prefix) carries
       out system dependent work. In particular, it checks the input
       stream so that in case of error, the >> -operator is terminated
       immediately. The reasons for this are explained in (Kreft and
       Langer, 1998). */

    const unsigned int maxi = 100;
    char buf[maxi];                 // buffer
    register size_t i = 0;

    // find beginning of word
    buf[0] = 0;
    while(is && !(isalpha(buf[0]) || '_' == buf[0]))
          is.get(buf[0]);

    /* When the beginning is found, all following underscores and
       alphanumeric characters are collected. `White space' or a
       special character terminate the reading process. */

    // collect the rest
    while(is &&  i < maxi
             && (isalnum(buf[i]) || '_' == buf[i]))
         is.get(buf[++i]);

    /* The last character read does not belong to the identifier. The
       iostream} library provides the possibility of returning an
       unused character to the input so that it is available to a
       subsequent program. */


    is.putback(buf[i]);   // back into the input stream
    buf[i] = char(0);

    /* Finally, a Null byte is inserted so that buf[] can be treated
       in the same way as a char*, and the buffer is passed to the
       private variable theIdentifier. */

    N.theIdentifier = buf;
    is.isfx();

    /* If some system dependent work must be undone, ifsx() ('s' for
       suffix) is called as a standard at the end of the operator (see
       (Kreft and Langer, 1998). */

    return is;
}

/* Implementation of the output operator is very easy; the internal
   string variable of an identifier is passed to the output os: */

ostream& operator<<(ostream& os, const Identifier& N)
{
    if(os.opfx())
    {
       os << N.toString();
       os.osfx();
    }
    return os;
}
