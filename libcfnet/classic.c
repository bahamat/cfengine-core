/*
   Copyright (C) CFEngine AS

   This file is part of CFEngine 3 - written and maintained by CFEngine AS.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; version 3.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA

  To the extent this program is licensed as part of the Enterprise
  versions of CFEngine, the applicable Commercial Open Source License
  (COSL) may apply to this file if you as a licensee so wish it. See
  included file COSL.txt.
*/


#include <classic.h>

#include <logging.h>
#include <misc_lib.h>

static bool LastRecvTimedOut(void)
{
#ifndef __MINGW32__
    if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
    {
        return true;
    }
#else
    int lasterror = GetLastError();

    if (lasterror == EAGAIN || lasterror == WSAEWOULDBLOCK)
    {
        return true;
    }
#endif

    return false;
}

/**
 * @brief Receive up to #toget bytes, plus a '\0', into buffer from sd.
 * @param sd Socket descriptor
 * @param buffer Buffer into which to read data
 * @param toget Number of bytes to read; a '\0' shall be written after
 *        the data; buffer must have space for that.
 *
 * @return number of bytes actually received, must be equal to #toget
 *         -1  in case of timeout or error - socket is unusable
 *         -1  also in case of early proper connection close
 *         0       NEVER
 *         <toget  NEVER
 */
int RecvSocketStream(int sd, char buffer[CF_BUFSIZE], int toget)
{
    int already, got;

    if (toget > CF_BUFSIZE - 1 || toget <= 0)
    {
        Log(LOG_LEVEL_ERR, "Bad software request to receive %d bytes", toget);
        return -1;
    }

    /* Repeat recv() until we get "toget" bytes. */
    for (already = 0; already < toget; already += got)
    {
        got = recv(sd, buffer + already, toget - already, 0);

        if (got == -1)
        {
            if (errno != EINTR)           /* recv() again in case of signal */
            {
                if (LastRecvTimedOut())
                {
                    Log(LOG_LEVEL_ERR, "Receive timeout"
                        " (received=%dB, expecting=%dB) (recv: %s)",
                        already, toget, GetErrorStr());
                    Log(LOG_LEVEL_VERBOSE,
                        "Consider increasing body agent control"
                        " \"default_timeout\" setting");

                    /* Shutdown() TCP connection despite of EAGAIN error, in
                     * order to avoid receiving this delayed response later on
                     * (Redmine #6027). */
                    shutdown(sd, SHUT_RDWR);
                }
                else
                {
                    Log(LOG_LEVEL_ERR, "Couldn't receive (recv: %s)",
                        GetErrorStr());
                }
                return -1;
            }
        }
        else if (got == 0)          /* peer has closed the connection early */
        {
            Log(LOG_LEVEL_ERR,
                "Peer closed connection early (received=%dB, expecting=%dB)",
                already, toget);
            buffer[already] = '\0';
            /* TODO Why return 0? Early close is an error so -1 is better. */
            return 0;
        }
    }
    assert(already == toget);

    buffer[already] = '\0';
    return already;
}

/*************************************************************************/

int SendSocketStream(int sd, const char buffer[CF_BUFSIZE], int tosend)
{
    int sent, already = 0;

    if (tosend <= 0)
    {
        Log(LOG_LEVEL_ERR, "Bad software request to send %d bytes",
            tosend);
        return -1;
    }

    EnforceBwLimit(tosend);

    do
    {
        sent = send(sd, buffer + already, tosend - already, 0);

        if ((sent == -1) && (errno == EINTR))
        {
            continue;
        }

        if (sent == -1)
        {
            Log(LOG_LEVEL_VERBOSE, "Couldn't send. (send: %s)", GetErrorStr());
            return -1;
        }

        already += sent;
    } while (already < tosend);

    return already;
}
