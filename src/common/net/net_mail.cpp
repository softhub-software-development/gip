
//
//  net_mail.cpp
//
//  Created by Christian Lehner on 07/15/19.
//  Copyright (c) 2019 Softhub. All rights reserved.
//

#include "stdafx.h"
#include "net_mail.h"

#define ALT_IMPL 0

#if ALT_IMPL
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
static const int READEND = 0;
static const int WRITEEND = 1;
#endif

#ifdef PLATFORM_MAC
#define SENDMAIL_CMD "/usr/sbin/sendmail -t"
#else
#define SENDMAIL_CMD "/usr/lib/sendmail -t"
#endif

using namespace SOFTHUB::BASE;
using namespace std;

namespace SOFTHUB {
namespace NET {

//
// class Mail_sender
//

int Mail_sender::send(const string& to, const string& subject, const string& body)
{
    int retval = -1;
#if defined PLATFORM_LINUX || defined PLATFORM_MAC
#if ALT_IMPL
    int pipe_fd[2];
    retval = pipe(pipe_fd);
    if (retval)
        return retval;
    pid_t child_pid = fork();
    if (child_pid < 0) {
        close(pipe_fd[READEND]);
        close(pipe_fd[WRITEEND]);
        return child_pid;
    } else if (!child_pid) {
        dup2(pipe_fd[READEND], READEND);
        close(pipe_fd[READEND]);
        close(pipe_fd[WRITEEND]);
        execlp("mail", "mail", "-s", subject.c_str(), to.c_str(), NULL);
        exit(EXIT_FAILURE);
    }
    close(pipe_fd[READEND]);
    retval = (int) write(pipe_fd[WRITEEND], body.c_str(), (int) body.size());
    if (retval < 0)
        return retval;
    close(pipe_fd[WRITEEND]);
    if (waitpid(child_pid, &retval, 0) == -1)
        return retval;
    return 0;
#else
    FILE* pipe = popen(SENDMAIL_CMD, "w");
    if (pipe) {
        const string& dfo = date_formatted();
        fprintf(pipe, "Subject: %s\n", subject.c_str());
        fprintf(pipe, "To: %s\n", to.c_str());
        fprintf(pipe, "Mailer: %s\n", "Softhub Atom Mailer");
        if (!mail_host.empty()) {
            const string& mid = date_time_message_id();
            fprintf(pipe, "Message-Id: <%s@%s>\n", mid.c_str(), mail_host.c_str());
        }
        fprintf(pipe, "Content-Type: text/plain; charset=\"utf-8\"");
        fprintf(pipe, "Date: %s\n", dfo.c_str());
        fprintf(pipe, "From: %s\n\n", sender.c_str());
        fwrite(body.c_str(), 1, body.length(), pipe);
        fwrite(".\n\004", 1, 3, pipe);
        retval = pclose(pipe);
     } else {
         perror("Failed to invoke sendmail");
     }
#endif
#elif defined PLATFORM_WIN
    // TODO
    cout << "sendmail not implemented" << endl;
/*
    char* command = "curl smtp://smtp.gmail.com:587 -v --mail-from \"SENDER.EMAIL@gmail.com\" --mail-rcpt \"RECEIVER.EMAIL@gmail.com\""
                    "--ssl -u SENDER.EMAIL@gmail.com:PASSWORD -T \"ATTACHMENT.FILE\" -k --anyauth";
    WinExec(command, SW_HIDE);
    return 0;
}
*/
#else
    cout << "sendmail not implemented" << endl;
#endif
#ifdef _DEBUG
    clog << "sent mail from " << sender << " to " << to << " retval: " << retval << endl;
#endif
    return retval;
}

string Mail_sender::date_time_message_id()
{
    char buf[128];
    time_t t = time(0);
    const struct tm* ts = gmtime(&t);
    assert(ts);
    // Example: 20190720062712.745101FA7A
    strftime(buf, sizeof(buf), "%Y%m%d%H.%S00000000", ts);
    return buf;
}

string Mail_sender::date_formatted()
{
    char buf[128];
    time_t t = time(0);
    const struct tm* ts = gmtime(&t);
    assert(ts);
    // Example Sat, 20 Jul 2019 06:27:12 +0000 (UTC)
    strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S +0000 (UTC)", ts);
    return buf;
}

}}
